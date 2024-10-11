#include<stdio.h>
#include<stdlib.h>
#include <inttypes.h>
#include<stdbool.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>

#define COUNT_RECORDS 10
#define MAXSIZE 256
#define FNAME "file"

struct record {
char name[80];
char address[80];
uint8_t semester;
};

FILE *fdopen(int handle, char *mode);

  struct record rec;
  struct record REC_SAV;
  struct record REC_NEW;
  int numRecord;
  int firstPosition, secondPosition;


struct record records[COUNT_RECORDS] = {
    {"Sarah Johnson", "11 Maple Ave", 3},
    {"Ethan Rodriguez", "098 Elm Rd", 1},
    {"Ava Nguyen", "83 Cedar Ln", 2},
    {"Lucas Hernandez", "15 Oak Blvd", 4},
    {"Isabella Morales", "41 Pine St", 2},
    {"Noah Ramirez", "88 Birch Ct", 1},
    {"Mia Diaz", "87 Walnut Dr", 3},
    {"Jacob Vargas", "009 Spruce Rd", 4},
    {"Abigail Flores", "647 Maple Ter", 2},
    {"William Castillo", "123 Elm Ct", 1}
};


void write_records(struct record *records) {
  FILE* f;
  if ((f = fopen("file", "wb")) == NULL) {
  printf("Не удалось открыть файл");
  exit(1);
  }
  fwrite(records, sizeof(struct record), COUNT_RECORDS, f);
  fclose(f);
}

void write_record(FILE* f, struct record rec) {
fwrite(&rec, sizeof(struct record), 1, f);
}

void print_record(struct record rec) {
  printf("%s ", rec.name);
    printf("%s ", rec.address);
      printf("%d\n", rec.semester);
}

void print_records() {
  FILE* f;
  int i;
  struct record readRecords[COUNT_RECORDS];
  if ((f = fopen("file", "rb")) == NULL) {
  printf("Не удалось открыть файл");
  exit(1);
  }
  printf("List of students: \n");
  size_t count = fread(readRecords, sizeof(struct record), COUNT_RECORDS,f);
  if(count < COUNT_RECORDS) {
    printf("empty\n");
  } else {
  for(i = 0; i < COUNT_RECORDS; i++) print_record(readRecords[i]);
  }
  fclose(f);
}


struct record readCurrentRecord(FILE* f) {
  struct record rec;
  fread(&rec, sizeof(struct record),1,f);
  return rec;
}

struct record get(int Rec_No) {
  FILE* f;
  struct record rec;
  f = fopen("file", "rb");
  fseek(f, Rec_No*sizeof(struct record), SEEK_SET);
  rec = readCurrentRecord(f);
  fclose(f);
  return rec;
}


void clear_file() {
  FILE* f = fopen("file", "wb");
  fclose(f);
}

void delay() {
  int value;
  printf("Нажмите любую клавишу для продолжения: ");
  scanf("%d", &value);
}

void menu(){
  printf("1 - VIEW all students\n");
  printf("2 - GET student bu number\n");
  printf("e - EXIT\n");
}

bool equals(struct record record1, struct record record2) {
  if (strcmp(record1.name, record2.name) != 0) {
         return false;
     }
  if (strcmp(record1.address, record2.address) != 0) {
         return false;
     }
  if (record1.semester != record2.semester) {
         return false;
     }
     return true;
 }

 struct record modificate(struct record rec) {
   int value;
   char c;
   while(1) {
 system("clear");
 print_record(rec);
 printf("'1' - change full name\n");
 printf("'2' - change address\n");
 printf("'3' - change semester\n");
 printf("'4' - cancel\n");
 scanf("%d", &value);
 switch(value) {
     case 1:
     {
       while ((c = getchar()) != '\n' && c != EOF) { }
       fgets(rec.name, 80-1, stdin);
       int length = strlen(rec.name);
       if (length > 0 && rec.name[length - 1] == '\n') {
       rec.name[length - 1] = '\0';
       break;
     }
     case 2:
     {
       while ((c = getchar()) != '\n' && c != EOF) { }
       fgets(rec.address, 80-1, stdin);
       int length = strlen(rec.address);
       if (length > 0 && rec.address[length - 1] == '\n') {
       rec.address[length - 1] = '\0';
   }
       break;
     }
     case 3:
     {
       scanf("%hhd", &rec.semester);
       break;
     }
     case 4: return rec;
     }
     }
   }
 }

void save(struct record rec) {
  int fd = open(FNAME, O_RDWR);
  FILE* f = fdopen(fd, "rb+");
  struct flock lock;
  REC_SAV = rec;
  rec = modificate(rec);
  fseek(f, (numRecord-1)*sizeof(struct record), SEEK_SET);
  firstPosition = ftell(f);
  fseek(f, sizeof(struct record), SEEK_CUR);
  secondPosition = ftell(f);
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = firstPosition;
  lock.l_len = secondPosition - firstPosition;
  if (fcntl(fd, F_SETLK, &lock) < 0)
  printf("Ошибка при:fcntl(fd, F_SETLK, F_WRLCK)(%s)\n",strerror(errno));
  fseek(f, firstPosition, SEEK_SET);
  REC_NEW = readCurrentRecord(f);
  if(!equals(REC_SAV, REC_NEW)) {
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) < 0)
    printf("Ошибка при:fcntl(fd, F_SETLK, F_UNLCK)(%s)\n",strerror(errno));
    printf("Someone changed this data\n");
    sleep(1);
    save(REC_NEW);
  } else {
    fseek(f, firstPosition, SEEK_SET);
    write_record(f, rec);
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) < 0)
    printf("Ошибка при:fcntl(fd, F_SETLK, F_UNLCK)(%s)\n",strerror(errno));
    delay();
  }
  fclose(f);
}



void menu2(struct record rec) {
  int value;
  printf("'1' - change data\n");
  printf("'2' - Exit\n");
  scanf("%d",&value);
  switch(value) {
    case 1:
    {
      save(rec);
      break;
    }
    case 2: break;
  }
}


int main() {
  char c;
  clear_file();
   while(1){
     menu();
     c = getchar();
    write_records(records);
     switch(c) {
       case '1':
       {
         system("clear");
         print_records();
         delay();
         break;
       }
       case '2':
       {
         system("clear");
         printf("Enter number student: ");
         scanf("%d", &numRecord);
         rec = get(numRecord-1);
         print_record(rec);
         menu2(rec);
         break;
       }
       case 'e': exit(0);
     }
     system("clear");
   }
  return 0;
}
