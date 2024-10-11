#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>

#define MAX_QUEUE_SIZE 15
#define MAX_CHILD_COUNT 5

sem_t *spare_msgs , *pick_msgs , *mutex;

pid_t consumers[MAX_CHILD_COUNT];
char* consumers_name[MAX_CHILD_COUNT];
int consumers_count;
pid_t producers[MAX_CHILD_COUNT];
char* producers_name[MAX_CHILD_COUNT];
int producers_count;

typedef struct{
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    char* data;
} message;

typedef struct{
    message* head;
    int h;
    message* tail;
    int t;
    message buff[MAX_QUEUE_SIZE];
    int count_added;
    int count_extracted;
}queue;

queue *message_queue;

uint8_t getSize(){
    int size = 0;
    while(size == 0)size = rand() % 257;
    if(size == 256)size=0;
    return size;
}

uint8_t getType(uint8_t size){
    if(size>128)return 1;
    else return 0;
}

char* getData(uint8_t size){
    if(size==0)size=(uint8_t)256;
    char* data = (char*) malloc(size);
    if(data==NULL)perror("queue message data");
    srand(time(NULL));
    for(int i = 0; i < size; i++)data[i] = (rand() % 26) + 'a';
    return data;
}

uint16_t FNV1_HASH(const void *data, size_t size) {
    const uint8_t *bytes = (const uint8_t *) data;
    uint16_t hash = 0x811C;
    for (size_t i = 0; i < size; ++i) {
        hash = (hash * 0x0101) ^ bytes[i];
    }
    return hash;
}

message* createMessage(){

    message* msg;
    msg = (message *) malloc(sizeof(*msg));
    msg->size = getSize();
    msg->data = getData(((msg->size + 3) / 4) * 4);
    msg->type = getType(msg->size);
    msg->hash = 0;
    msg->hash = FNV1_HASH(msg->data, strlen(msg->data));

    return msg;
}

void start() {
    int fd = shm_open("/queue", (O_RDWR | O_CREAT | O_TRUNC), (S_IRUSR | S_IWUSR));
    if (fd < 0) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(fd, sizeof(queue))) {
        perror("message_queue");
        exit(1);
    }

    void *ptr = mmap(NULL, sizeof(queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("map");
        exit(1);
    }

    message_queue = (queue *) ptr;

    if (close(fd)) {
        perror("close");
        exit(1);
    }

    memset(message_queue, 0, sizeof(queue));

    mutex = sem_open("mutex", (O_RDWR | O_CREAT | O_TRUNC), (S_IRUSR | S_IWUSR), 1);
    spare_msgs = sem_open("spare_msgs_sem", O_CREAT, 0666, MAX_QUEUE_SIZE);
    pick_msgs = sem_open("pick_msgs_sem", O_CREAT, 0666, 0);

    if (spare_msgs == SEM_FAILED || pick_msgs == SEM_FAILED || mutex == SEM_FAILED) {
        perror("spare_msgs/pick_msgs/mutex semaphore");
        exit(1);
    }
}

void deleteConsumers(){
    if(consumers_count == 0){
        printf("No consumers.");
        return;
    }
    consumers_count--;
    kill(consumers[consumers_count],SIGKILL);
    printf("Was delete consumer with name:%s , pid:%d\n",consumers_name[consumers_count],consumers[consumers_count]);
    free(consumers_name[consumers_count]);
}

void deleteProducers(){
    if(producers_count == 0){
        printf("No consumers.");
        return;
    }
    producers_count--;
    kill(producers[producers_count],SIGKILL);
    printf("Was delete producer with name:%s , pid:%d\n",producers_name[producers_count],producers[producers_count]);
    free(producers_name[producers_count]);
}

void fromProgExit(){
    while (consumers_count) deleteConsumers();
    while (producers_count) deleteProducers();

    munmap(message_queue, sizeof(queue));

    shm_unlink("/queue");

    sem_close(mutex);

    sem_close(spare_msgs);

    sem_close(pick_msgs);

    sem_unlink("mutex");

    sem_unlink("spare_msgs_sem");

    sem_unlink("pick_msgs_sem");

    exit(0);
}

void viewStatus(){

    printf("-----------------------------\n");
    printf("Queue max size:%d\n",MAX_QUEUE_SIZE);
    printf("Current size:%d\n",message_queue->count_added-message_queue->count_extracted);
    printf("Added:%d \nExtracted:%d\n",message_queue->count_added,message_queue->count_extracted);
    printf("Consumers:%d\n",consumers_count);
    printf("Producers:%d\n",producers_count);
    printf("-----------------------------\n");
}

int addMessage(message* msg){
    sem_wait(spare_msgs);
    sem_wait(mutex);

    if(message_queue->count_added-message_queue->count_extracted==MAX_QUEUE_SIZE){
        printf("Cannot add message queue is full.\n");
        return -1;
    }

    if(message_queue->count_added-message_queue->count_extracted==0){
        message_queue->buff[0] = *msg;
        message_queue->head = message_queue->tail = &message_queue->buff[0];
    }else message_queue->t++;

    message_queue->count_added++;
    message_queue->t = message_queue->t%MAX_QUEUE_SIZE;
    message_queue->buff[message_queue->t]=*msg;
    message_queue->tail = &message_queue->buff[message_queue->t];

    sem_post(pick_msgs);
    sem_post(mutex);
    return message_queue->count_added;
}

int extractedMessage(message **msg){
    sem_wait(pick_msgs);
    sem_wait(mutex);

    if(message_queue->count_added == message_queue->count_extracted){
        printf("Queue is empty.");
        return -1;
    }
    *msg = message_queue->head;
    message_queue->count_extracted++;
    message_queue->h++;
    message_queue->h = message_queue->h % MAX_QUEUE_SIZE;
    message_queue->head = &message_queue->buff[message_queue->h];

    sem_post(spare_msgs);
    sem_post(mutex);
    return message_queue->count_extracted;
}

void addConsumer(){
    if(consumers_count==MAX_CHILD_COUNT){
        printf("The maximum number of consumers has already been created.");
        return;
    }
    pid_t local_pid = fork();
    if(local_pid == -1)perror("fork");
    else if(local_pid == 0){
        message* msg;
        char name[20];
        sprintf(name, "consumer_%02d", consumers_count);
        while(1){
            int ret = extractedMessage(&msg);
            if(ret != -1){
                printf("%s consumer message: HASH=%04X, counter_extracted=%d\n", name, msg->hash, ret);
            }
            sleep(5);
        }
    }
    consumers[consumers_count] = local_pid;
    consumers_name[consumers_count] = (char *) malloc(16);
    sprintf(consumers_name[consumers_count], "consumer_%02d", consumers_count);
    consumers_count++;
}

void addProducer(){
    if(producers_count==MAX_CHILD_COUNT){
        printf("The maximum number of producers has already been created.");
        return;
    }
    pid_t local_pid = fork();
    if(local_pid == -1)perror("fork");
    else if(local_pid == 0){
        message* msg;
        char name[20];
        sprintf(name, "producer%02d", producers_count);
        while(1){
            msg = createMessage();
            int ret = addMessage(msg);
            if(ret != -1){
                printf("%s producer message: HASH=%04X, counter_added=%d\n", name, msg->hash, ret);
            }
            sleep(5);
        }
    }
    producers[producers_count] = local_pid;
    producers_name[producers_count] = (char *) malloc(16);
    sprintf(producers_name[producers_count], "producer%02d", producers_count);
    producers_count++;
}

void menu() {
    printf("------------------------------\n");
    printf("'1' - to create producer.\n");
    printf("'2' - to remove producer.\n");
    printf("'3' - to create consumer.\n");
    printf("'4' - to remove consumer.\n");
    printf("'p' - viev processes.\n");
    printf("'s' - viev status.\n");
    printf("'e' - to exit.\n");
    printf("------------------------------\n");
}

void viewProcesses(){
    if(consumers_count == 0 && producers_count == 0){
        printf("no processes running\n");
        return;
    }
    for(int i = 0; i < consumers_count; i++) printf("%s , %d\n",consumers_name[i],consumers[i]);
    for(int i = 0; i < producers_count; i++) printf("%s , %d\n",producers_name[i],producers[i]);
}

int main(){
start();

    printf("Write 'm' to display menu.\n");

    while (1) {
        char sumbol;
        rewind(stdin);
        sumbol = getchar();
        switch (sumbol) {
            case '1': {
                addProducer();
                break;
            }
            case '2': {
                deleteProducers();
                break;
            }
            case '3': {
                addConsumer();
                break;
            }
            case '4': {
                deleteConsumers();
                break;
            }
            case 'p': {
                viewProcesses();
                break;
            }
            case 'e': {
                fromProgExit();
                break;
            }
            case 's': {
                viewStatus();
                break;
            }
            case 'm': {
                menu();
                break;
            }
            default:
                break;
        }
    }
fromProgExit();
    return 0;
}
