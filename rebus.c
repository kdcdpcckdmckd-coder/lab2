#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define MAXW 8
#define MAXL 16

char words[MAXW][MAXL];
int nadd;
char letters[10];
int nlet;
int used[10];
int val[256];
int found;

void parse(const char *s) {
    nadd = 0;
    nlet = 0;
    int wi = 0, pos = 0;
    int seen[256] = {0};

    for (int i = 0; s[i]; i++) {
        char c = s[i];
        if (isalpha((unsigned char)c)) {
            words[wi][pos++] = toupper((unsigned char)c);
        } else if (c == '+' || c == '=') {
            if (pos) {
                words[wi][pos] = 0;
                wi++;
                pos = 0;
            }
        }
    }
    if (pos) {
        words[wi][pos] = 0;
        wi++;
    }
    nadd = wi - 1;

    for (int w = 0; w < wi; w++)
        for (int i = 0; words[w][i]; i++) {
            unsigned char ch = words[w][i];
            if (!seen[ch]) {
                seen[ch] = 1;
                letters[nlet++] = ch;
            }
        }
}

long wval(const char *w) {
    long v = 0;
    for (int i = 0; w[i]; i++)
        v = v * 10 + val[(unsigned char)w[i]];
    return v;
}

int badzero(void) {
    for (int w = 0; w <= nadd; w++) {
        int len = strlen(words[w]);
        if (len > 1 && val[(unsigned char)words[w][0]] == 0)
            return 1;
    }
    return 0;
}

int checksum(void) {
    long s = 0;
    for (int w = 0; w < nadd; w++)
        s += wval(words[w]);
    return s == wval(words[nadd]);
}

void go(int idx) {
    if (found) return;

    if (idx == nlet) {
        if (!badzero() && checksum())
            found = 1;
        return;
    }

    char l = letters[idx];
    for (int d = 0; d <= 9 && !found; d++) {
        if (used[d]) continue;
        used[d] = 1;
        val[(unsigned char)l] = d;
        go(idx + 1);
        used[d] = 0;
    }
}

int solve(const char *in, char *out, size_t sz) {
    parse(in);
    if (nlet > 10) return 0;

    memset(used, 0, sizeof(used));
    memset(val, -1, sizeof(val));
    found = 0;

    go(0);
    if (!found) return 0;

    char tmp[256] = {0};
    int off = 0;
    for (int w = 0; w < nadd; w++) {
        off += snprintf(tmp + off, sz - off, "%ld", wval(words[w]));
        if (w != nadd - 1)
            off += snprintf(tmp + off, sz - off, " + ");
    }
    snprintf(tmp + off, sz - off, " = %ld", wval(words[nadd]));

    strncpy(out, tmp, sz - 1);
    out[sz - 1] = 0;
    return 1;
}

int main(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    char in[256], out[256];
    printf("Введите ребус (например: SEND + MORE = MONEY): ");
    if (!fgets(in, sizeof(in), stdin)) return 1;
    in[strcspn(in, "\n")] = 0;

    if (solve(in, out, sizeof(out)))
        printf("Решение: %s\n", out);
    else
        printf("Решение не найдено.\n");

    return 0;
}
