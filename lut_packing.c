#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_LUTS 12000
#define MAX_INPUTS 6
#define MAX_NAME_LEN 64

typedef struct{
    char output [MAX_NAME_LEN];
    char inputs [MAX_INPUTS][MAX_NAME_LEN];
    int num_inputs;
} LUT; //define a LUT as arrays of inputs, outputs and number of inputs

LUT luts[MAX_LUTS];
int lut_count = 0;

int g[MAX_LUTS][MAX_LUTS]; //list storage [v][0...deg[v]-1]
int deg[MAX_LUTS];
int N;

int match_arr[MAX_LUTS];
int base_arr[MAX_LUTS];
int p_arr[MAX_LUTS];
int q_arr[MAX_LUTS];
int used_arr[MAX_LUTS];
int blossom_arr[MAX_LUTS];
int n_global;

int lowest_common_ancestor(int a, int b){
    static int used_path[MAX_LUTS];
    memset(used_path, 0, sizeof(int)*n_global);

    while(1){
        a = base_arr[a];
        used_path[a] = 1;
        if (match_arr[a] == -1)
            break;
        a = p_arr[match_arr[a]];
    }
    while (1) {
        b = base_arr[b];
        if (used_path[b])
            return b;
        b = p_arr[match_arr[b]];
    }
}

void mark_path(int v, int b, int children){
    while (base_arr[v] != b) {
        blossom_arr[base_arr[v]] = blossom_arr[base_arr[match_arr[v]]] = 1;
        p_arr[v] = children;
        children = match_arr[v];
        v = p_arr[match_arr[v]];
    }
}

//bfs search for a path starting from root
int find_path(int root){
    int qh = 0, qt = 0;
    int v, u;
    int i;

    memset(used_arr, 0, sizeof(int)*n_global);
    memset(p_arr, -1, sizeof(int)*n_global);

    for(i = 0; i < n_global; i++){
        base_arr[i] = i;
    }

     q_arr[qt++] = root;
     used_arr[root] = 1;

     while(qh < qt){
         v = q_arr[qh++];
         for(i = 0; i < deg[v]; i++){
             u = g[v][i];
             if (base_arr[v] == base_arr[u] || match_arr[v] == u) continue;

             if (u == root || (match_arr[u] != -1 && p_arr[ match_arr[u] ] != -1)) {
                 int curbase = lowest_common_ancestor(v, u);
                 int j;
                 memset(blossom_arr, 0, sizeof(int)*n_global);
                 mark_path(v, curbase, u);
                 mark_path(u, curbase, v);
                 for (j = 0; j < n_global; ++j) {
                     if (blossom_arr[ base_arr[j] ]) {
                         base_arr[j] = curbase;
                         if (!used_arr[j]) {
                             used_arr[j] = 1;
                             q_arr[qt++] = j;
                         }
                     }
                 }
             }else if (p_arr[u] == -1) {
                 p_arr[u] = v;
                 if (match_arr[u] == -1) {
                     int cur = u;
                     while (cur != -1) {
                         int prev = p_arr[cur];
                         int next = (prev == -1) ? -1 : match_arr[prev];
                         match_arr[cur] = prev;
                         if (prev != -1) match_arr[prev] = cur;
                         cur = next;
                     }
                     return 1;
                 }else{
                     used_arr[ match_arr[u] ] = 1;
                     q_arr[qt++] = match_arr[u];
                 }
             }
         }
     }
     return 0;
}

//fills match_arr and returns the number of pairs
int maximum_matching(int n){
    int i;
    int pairs = 0;
    n_global = n;
    memset(match_arr, -1, sizeof(int) * n_global);

    for (i = 0; i < n_global; i++) {
        if (match_arr[i] == -1) {
            find_path(i);
        }
    }
    for (i = 0; i < n_global; i++) {
        if (match_arr[i] != -1)
            pairs++;
    }
    return pairs / 2;
}

int compatibility_check(const LUT *a, const LUT *b){
    int unique = a->num_inputs;
    int i, j;
    int found;

    if (a->num_inputs > 5 || b->num_inputs > 5) return 0; //5+ inputs cannot share a 6-LUT

    //counts each input (a) as unique and adds any input (b) that is not a unique input
    for (i = 0; i < b->num_inputs; i++) {
        found = 0;
        for(j = 0; j < a->num_inputs; j++){
            if (strcmp(b->inputs[i], a->inputs[j]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            unique++;
        }
        if (unique > 5) { //cannot share 6-LUT
            return 0;
        }
    }
    return 1;
}

//extract LUTS for .names line
int add_lut_from_names(char *line){
    if (lut_count >= MAX_LUTS) return 1;

    char *token;
    char *tokens[1 + MAX_INPUTS];
    int tcount = 0;
    int i;

    token = strtok(line, " \t\r\n");
    while ((token = strtok(NULL, " \t\r\n")) != NULL) {
        tokens[tcount++] = token;
    }

    if(tcount < 1) return 1;

    LUT *lut = &luts[lut_count];
    memset(lut, 0, sizeof(LUT));

    strncpy(lut->output, tokens[tcount - 1], MAX_NAME_LEN - 1);
    lut->output[MAX_NAME_LEN - 1] = '\0';

    lut->num_inputs = tcount - 1;

    if (lut->num_inputs > MAX_INPUTS)
        lut->num_inputs = MAX_INPUTS;

    for (i = 0; i < lut->num_inputs; ++i) {
        strncpy(lut->inputs[i], tokens[i], MAX_NAME_LEN - 1);
        lut->inputs[i][MAX_NAME_LEN - 1] = '\0';
    }

    lut_count++;
    return 0;
}

void parse_blif_file(FILE *f){
    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (strncmp(p, ".names", 6) == 0 && (p[6] == ' ' || p[6] == '\t' || p[6] == '\n' || p[6] == '\r')) { //only parsing the .names lines
            add_lut_from_names(p);
        }
    }
}

int main(int argc, char **argv){
    FILE *f = stdin;
    int i, j;
    if (argc >= 2) {
        f = fopen(argv[1], "r");
        if (!f) {
            perror("Error opening input file");
            return 1;
        }
    }

    parse_blif_file(f);
    if (f != stdin)
        fclose(f);

    N = lut_count;
    //building compatibility graphs
    for (i = 0; i < N; i++)
        deg[i] = 0;
    for (i = 0; i < N; i++) {
        for (j = i + 1; j < N; j++) {
            if (compatibility_check(&luts[i], &luts[j])) {
                g[i][deg[i]++] = j;
                g[j][deg[j]++] = i;
            }
        }
    }

    int pairs = maximum_matching(N);
    int printed[MAX_LUTS] = {0};

    for (i = 0; i < N; i++) {
        if (printed[i])
            continue;
        if (match_arr[i] != -1 && !printed[ match_arr[i] ]) {
            int j = match_arr[i];
            printf("%s %s\n", luts[i].output, luts[j].output);
            printed[i] = printed[j] = 1;
        } else {
            printf("%s\n", luts[i].output); //unmatched lut assigned individual 6-lut
            printed[i] = 1;
        }
    }

    fprintf(stderr, "Total LUTs: %d, pairs: %d, packed 6-LUTs: %d\n", N, pairs, pairs + (N - 2*pairs));
    return 0;
}
