#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <ctype.h>
#define max_size_input 10000
#define max_size_word 46

int replace = 0;

typedef struct {
    GtkWidget *entry;
    GtkWidget *text_view;
} EntryAndTextView;

int checkWord(wchar_t *word, int lang)
{
    wchar_t *temp, original = 0;
    FILE *dict;

    setlocale(LC_ALL, "en_US.UTF-8");

    temp = (wchar_t *) calloc(46, sizeof(wchar_t));
    if(!temp)
    {
        fprintf(stderr, "Невозможно открыть файл\n");
        return -1;
    }

    if(lang)
        dict = fopen("./english.txt", "r");
    else
        dict = fopen("./russian.txt", "r");

    if(dict == NULL)
    {
        fprintf(stderr, "Невозможно открыть файл\n");
        return -1;
    }

    original = word[0];

    while(fgetws(temp, max_size_word, dict))
    {
        if(feof(dict))
            break;

        temp[wcscspn(temp, L"\n")] = '\0';
        temp[wcscspn(temp, L"\r")] = '\0';
        temp[wcscspn(temp, L"%")] = '\0';

        word[0] = tolower(word[0]);

        if(!wcscmp(word, temp))
        {
            word[0] = original;
            return 1;
        }
    }

    word[0] = original;
    fclose(dict);

    return 0;
}

void switchLayout()
{
    XkbStateRec state;
    int group;

    Display *display = XOpenDisplay(NULL);
    if(display == NULL)
    {
        fprintf(stderr, "Невозможно открыть дисплей\n");
        return;
    }

    XkbGetState(display, XkbUseCoreKbd, &state);    //получить текущее состояние клавиатуры
    group = (state.group + 1) % XkbNumKbdGroups;
    XkbLockGroup(display, XkbUseCoreKbd, group);    //переключить следующую раскладку

    XCloseDisplay(display);
}


void change_layout(GtkWidget *widget, gpointer data) {
    g_print("Раскладка поменялась!\n");
    switchLayout();
}

void on_button_clicked(GtkWidget *widget, gpointer data)
{
    EntryAndTextView *widgets = (EntryAndTextView *)data;

    const gchar *text = gtk_entry_get_text(GTK_ENTRY(widgets->entry));

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->text_view));

    gtk_text_buffer_set_text(buffer, "", -1);

    // Добавляем обработку проверки орфографии
    wchar_t *input, *changed_input, *word, *ptr, separators[] = L" ,.?!";
    long int start = 0;
    void checkSpelling(wchar_t *);

    setlocale(LC_ALL, "en_US.UTF-8");

    input = (wchar_t *) calloc(max_size_input, sizeof(wchar_t));
    if(!input)
    {
        fprintf(stderr, "Память не выделена\n");
        return -1;
    }

    changed_input = (wchar_t *) calloc(max_size_input, sizeof(wchar_t));
    if(!changed_input)
    {
        fprintf(stderr, "Память не выделена\n");
        return -1;
    }

    ptr = (wchar_t *) calloc(max_size_input, sizeof(wchar_t));
    if(!ptr)
    {
        fprintf(stderr, "Память не выделена\n");
        return -1;
    }

    word = (wchar_t *) calloc(max_size_input, sizeof(wchar_t));
    if(!word)
    {
        fprintf(stderr, "Память не выделена\n");
        return -1;
    }

    system("clear");

    // Заменяем ввод текста через fgetws на получение текста из *text
    mbstowcs(input, text, strlen(text) + 1);

    switchLayout();

    wcscpy(changed_input, input);

    word = wcstok(input, separators, &ptr);
    while(word != NULL)
    {
        start = word - input;

        checkSpelling(word);

        if(replace)
            for(size_t i=start, j=0; j<wcslen(word); i++, j++)
                changed_input[i] = word[j];

        word = wcstok(NULL, separators, &ptr);
    }

    setlocale(LC_ALL, "en_US.UTF-8");

    // Заменяем вывод текста через wprintf на вставку текста в buffer
    char *output = (char *) calloc(max_size_input, sizeof(char));
    if(!output)
    {
        fprintf(stderr, "Память не выделена\n");
        return -1;
    }

    wprintf(L"%ls\n", changed_input);
    gchar* utf8_input = g_ucs4_to_utf8(changed_input, -1, NULL, NULL, NULL);
    gtk_text_buffer_insert_at_cursor(buffer, utf8_input, -1);
    g_free(utf8_input);

}

void checkSpelling(wchar_t *word)
{
    int english = 0, russian = 0;
    void replaceFragment(wchar_t *, int);

    for(size_t i=0; i<wcslen(word); i++)
    {
        setlocale(LC_ALL, "");

        if((isalpha(word[i]) && (word[i] >= 'a' && word[i] <= 'z')) || (word[i] >= 'A' && word[i] <= 'Z'))
            english++;

        else
            russian++;
    }

    if(english && !russian)
    {
        if(checkWord(word, 1))
        {
            replace = 0;
            return;
        }
        else
        {
            replace = 1;
            replaceFragment(word, 1);
        }
    }

    else if(!english && russian)
    {
        if(checkWord(word, 0))
        {
            replace = 0;
            return;
        }
        else
        {
            replace = 1;
            replaceFragment(word, 0);
        }
    }

    else if(russian && english)
    {
        replace = 1;

        replaceFragment(word, 0);
        if(!checkWord(word, 1))
        {
            replaceFragment(word, 1);
            checkWord(word, 0);
        }
    }
}

void replaceFragment(wchar_t *word, int lang)
{
    wchar_t en_layout[] = L"`qwertyuiop[]asdfghjkl;'zxcvbnm,.`QWERTYUIOP[]ASDFGHJKL;'ZXCVBNM,.",
    ru_layout[] =         L"ёйцукенгшщзхъфывапролджэячсмитьбюЁЙЦУКЕНГШЩЗХЪФЫВАПРОЛДЖЭЯЧСМИТЬБЮ";

    setlocale(LC_ALL, "en_US.UTF-8");

    for(size_t i=0; i<wcslen(word); i++)
    {
        if(lang == 1)
        {
            for(size_t j=0; j<wcslen(en_layout); j++)
                if(word[i] == en_layout[j])
                {
                    word[i] = ru_layout[j];
                    break;
                }
        }

        else if(lang == 0)
        {
            for(size_t j=0; j<wcslen(ru_layout); j++)
                if(word[i] == ru_layout[j])
                {
                    word[i] = en_layout[j];
                    break;
                }
        }
    }
}

void on_copy_button_clicked_full_translete(GtkWidget *widget, gpointer data)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, text, -1);

    g_free(text);
}

void clear_button_clicked(GtkWidget *widget, EntryAndTextView *widgets) {
    gtk_entry_set_text(GTK_ENTRY(widgets->entry), "");
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->text_view));
    gtk_text_buffer_set_text(buffer, "", -1);
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "ПРОГРАММА СТОРОЖ");
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);
    gtk_widget_set_size_request(window, 300, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 7);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *layout_button = gtk_button_new_with_label("Изменить раскладку");
    g_signal_connect(layout_button, "clicked", G_CALLBACK(change_layout), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), layout_button, FALSE, FALSE, 0);

    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), text_view, TRUE, TRUE, 0);

    EntryAndTextView *widgets = g_new(EntryAndTextView, 1);
    widgets->entry = entry;
    widgets->text_view = text_view;

    GtkWidget *result_button = gtk_button_new_with_label("Посмотреть результат");
    g_signal_connect(result_button, "clicked", G_CALLBACK(on_button_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(vbox), result_button, FALSE, FALSE, 0);

    GtkWidget *copy_button = gtk_button_new_with_label("COPY");
    g_signal_connect(copy_button, "clicked", G_CALLBACK(on_copy_button_clicked_full_translete), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), copy_button, FALSE, FALSE, 0);

    GtkWidget *clear_button = gtk_button_new_with_label("Очистить");
    g_signal_connect(clear_button, "clicked", G_CALLBACK(clear_button_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(vbox), clear_button, FALSE, FALSE, 0);

    GtkWidget *exit_button = gtk_button_new_with_label("EXIT");
    g_signal_connect(exit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), exit_button, FALSE, FALSE, 0);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

