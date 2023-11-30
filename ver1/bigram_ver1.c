#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 

char* readFile(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("File opening failed");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    size_t bytesRead = fread(string, 1, fsize, file);
    if (bytesRead != fsize) {
        fprintf(stderr, "File read error\n");
        free(string);
        fclose(file);
        return NULL;
    }

    fclose(file);
    string[fsize] = 0;
    return string;
}


void lower1(char* str) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] - 'A' + 'a';
        }
    }
}


int hashFunction(const char* bigram, int s) {
    unsigned long hash = 5381;
    int c;
    while (c = *bigram++) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash % s;
}

typedef struct Node {
    char bigram[128]; // 두 단어의 bigram을 저장하기 위한 충분한 크기
    int frequency;
    struct Node* next;
} Node;


typedef struct {
    Node** buckets;
    int size;
} HashTable;

HashTable* createHashTable(int size) {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    table->size = size;
    table->buckets = (Node**)malloc(sizeof(Node*) * size);
    for (int i = 0; i < size; i++) {
        table->buckets[i] = NULL;
    }
    return table;
}



void addBigramToHashTable_recurcive(HashTable* table, const char* bigram) {
    int index = hashFunction(bigram, table->size);
    Node* current = table->buckets[index];

    // 기존 노드에서 bigram을 찾아 빈도수를 업데이트
    while (current != NULL) {
        if (strcmp(current->bigram, bigram) == 0) {
            current->frequency++;
            return;
        }
        current = current->next;
    }

    // 새 노드 생성
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed for new Node.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(newNode->bigram, bigram);
    newNode->frequency = 1;
    newNode->next = table->buckets[index];
    table->buckets[index] = newNode;
}

int countNodesInHashTable(HashTable* table) {
    int count = 0;
    for (int i = 0; i < table->size; i++) {
        for (Node* current = table->buckets[i]; current != NULL; current = current->next) {
            count++;
        }
    }
    return count;
}


Node** extractNodesFromHashTable(HashTable* table) {
    int count = countNodesInHashTable(table);
    Node** array = (Node**)malloc(count * sizeof(Node*));
    int index = 0;

    for (int i = 0; i < table->size; i++) {
        for (Node* current = table->buckets[i]; current != NULL; current = current->next) {
            array[index++] = current;
        }
    }
    return array;
}

void processTextToWordBigrams(HashTable* table, const char* text) {
    const char* delimiters = " \t\n.,;:!?'\"-()"; // 단어를 분리할 구분자
    char* textCopy = strdup(text); // text 복사본 생성
    char* word = strtok(textCopy, delimiters); // 첫 번째 단어

    while (word != NULL) {
        char* nextWord = strtok(NULL, delimiters); // 다음 단어
        if (nextWord == NULL) {
            break;
        }

        char bigram[128]; // 두 단어를 저장할 충분한 공간
        snprintf(bigram, sizeof(bigram), "%s %s", word, nextWord); // 두 단어를 결합하여 bigram 생성
        addBigramToHashTable_recurcive(table, bigram);

        word = nextWord; // 다음 bigram을 위해 현재 단어 업데이트
    }

    free(textCopy); // 복사된 문자열 메모리 해제
}


void bubbleSort(Node** array, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (array[j]->frequency < array[j + 1]->frequency) {
                Node* temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}


void printBigramsToFile(Node** array, int n, const char* filename) {
    FILE* file = fopen(filename, "w"); // 파일을 쓰기 모드로 열기
    if (file == NULL) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "Bigram Frequency\n");
    for (int i = 0; i < n; i++) {
        fprintf(file, "%s: %d\n", array[i]->bigram, array[i]->frequency);
    }

    fclose(file); // 파일 닫기
}

void deleteHashTable(HashTable* table) {
    for (int i = 0; i < table->size; i++) {
        Node* temp = table->buckets[i];
        while (temp != NULL) {
            Node* toDelete = temp;
            temp = temp->next;
            free(toDelete);
        }
    }
    free(table->buckets);
    free(table);
}


// 메인 함수 예제
int main() {
    const char* filename = "670kb.txt";
    char* content = readFile(filename);

    if (content == NULL) {
        return 1;
    }

    lower1(content); // 혹은 lower2(content);
    // lower2(content);

    // 해시 테이블 생성
    int tableSize = 100;
    HashTable* table = createHashTable(tableSize);
    
    // 텍스트를 bigram으로 변환하여 해시 테이블에 추가
    processTextToWordBigrams(table, content);

    // 해시 테이블에서 노드 추출 및 정렬
    int nodeCount = countNodesInHashTable(table);
    Node** nodes = extractNodesFromHashTable(table);
    bubbleSort(nodes, nodeCount);

    // 파일에 정렬된 배열 출력
    printBigramsToFile(nodes, nodeCount, "output.txt");

    // 메모리 해제
    free(nodes);
    deleteHashTable(table);
    free(content);

    return 0;
}

