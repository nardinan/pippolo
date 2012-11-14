/*
    example 1 - insert some records into the database
 */
#include "pippolo_api.h"
#define example_temporary_length 32
pthread_mutex_t counter;
unsigned int value;
char names[][example_temporary_length] = {
    "bruno",
    "filippo",
    "luca",
    "tullio",
    "roberto",
    "marco",
    "carlo",
    "paolo",
    "alessio",
    "emanuele",
    "ilario",
    "vincenzo",
    "maurizio",
    "renato",
    "guido",
    "pietro",
    "luna",
    "stefania",
    "marta",
    "oriana",
    "daniela",
    "ilaria",
    "angela",
    "giulia",
    "ave",
    "teresa",
    "rosetta",
    "elisa",
    "NULL"
};
char surnames[][example_temporary_length] = {
    "capezzali",
    "paolucci",
    "caprini",
    "guli",
    "crocioni",
    "lanciotti",
    "pigliautile",
    "pelliccia",
    "nardinocchi",
    "gentili",
    "mariani",
    "paciucci",
    "betti",
    "lezzerini",
    "cardianli",
    "baroni",
    "vallorani",
    "di giambattista",
    "toffoli",
    "NULL"
};

void hooker (const char *ID, struct str_xml_node *result) {
    int index = 0;
    struct str_xml_node *records, *columns;
    struct str_xml_key *key;
    if (p_string_cmp(ID, "research") == 0) {
        records = result->children;
        while (records) {
            index++;
            printf("[");
            columns = records->children;
            while (columns) {
                key = columns->keys;
                while (key) {
                    if (p_string_cmp(key->key, pippolo_xml_key_key) == 0) {
                        printf("{%s: ", key->value);
                        break;
                    }
                    key = key->next;
                }
                printf("\"%s\"}", columns->value);
                columns = columns->next;
            }
            printf("]\n");
            records = records->next;
        }
        printf("[records: %d]\n", index);
    } else
        printf("operation completed successfully!\n");
    pippolo_lock_action(counter, (value--));
}

int discretizer (const char *value) {
    int result = 0;
    char *ptr = (char *)value;
    while (*ptr)
        result += *ptr++;
    result = (result%10);
    return result;
}

int main (int argc, char *argv[]) {
    pthread_mutex_init(&counter, NULL);
    struct str_record *records = NULL;
    int terminated = pippolo_false, subindex;
    size_t nlen = 0, slen = 0;
    char backup[example_temporary_length];
    p_node_pippolo_init("marcello");
    p_node_pippolo_add("127.0.0.1", 5090);
    p_node_pippolo_add("127.0.0.1", 7090);
    p_node_pippolo_add("127.0.0.1", 4090);
    pippolo_discretizer = &discretizer;
    /* 
        ADDER
            this piece of code create (randomly) new elements to add
            into the database.
     */
    while (p_string_cmp(names[nlen], "NULL") != 0)
        nlen++;
    while (p_string_cmp(surnames[slen], "NULL") != 0)
        slen++;
    for (subindex = 0; subindex < 24; subindex++) {
        p_node_record_add(&records);
        snprintf(backup, example_temporary_length, "%d", rand());
        p_node_record_keys_add(&records, "fiscal", backup, pippolo_true);
        p_node_record_keys_add(&records, "name", names[rand()%nlen], pippolo_false);
        p_node_record_keys_add(&records, "surname", surnames[rand()%slen], pippolo_false);
    }
    pippolo_lock_action(counter, (value++));
    p_node_action("insert", EDATA_ACTIONS_ADD, records, 10, &hooker);
    /*
        REQUESTER
            this piece of code serach into the database for matching
            elements (using regex)
     */
    /*p_node_record_add(&records);
    p_node_record_keys_add(&records, "name", "^[a-z]", pippolo_false);
    pippolo_lock_action(counter, (value++));
    p_node_action("research", EDATA_ACTIONS_GET, records, 10, &hooker);*/
    /* 
        ERASER
            this piece of code remove matching elements from database
            (using regex)
     */
    /*p_node_record_add(&records);
    p_node_record_keys_add(&records, "name", "^[a-z]", pippolo_false);
    pippolo_lock_action(counter, (value++));
    p_node_action("delete", EDATA_ACTIONS_DELETE, records, 10, &hooker);*/
    while (pippolo_true) {
        p_wait(0, 500000);
        pippolo_lock_action(counter, (terminated=(value==0)));
        if (terminated)
            break;
    }
    p_node_records_destroy(&records);
    p_node_pippolo_quit();
    pthread_mutex_destroy(&counter);
    return 0;
}