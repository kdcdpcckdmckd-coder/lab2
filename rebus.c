#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define MAXW 8
#define MAXL 16

char words[MAXW][MAXL];
char ops[MAXW];
int nadd;
int val[256];
int used[10];
int is_leading[256];
int found;
int maxlen;
char letters[10];
int nlet;
int only_plus;

void parse(const char *s) {
    nadd = 0;
    nlet = 0;
    int wi = 0, pos = 0, oc = 0;
    int seen[256] = {0};

    for (int i = 0; s[i]; i++) {
        char c = s[i];
        if (isalpha((unsigned char)c)) {
            words[wi][pos++] = toupper((unsigned char)c);
        } else if (c == '+' || c == '-' || c == '*' || c == '/') {
            if (pos) { words[wi][pos] = 0; wi++; pos = 0; }
            ops[oc++] = c;
        } else if (c == '=') {
            if (pos) { words[wi][pos] = 0; wi++; pos = 0; }
        }
    }
    if (pos) { words[wi][pos] = 0; wi++; }
    nadd = wi - 1;

    only_plus = 1;
    for (int i = 0; i < nadd - 1; i++)
        if (ops[i] != '+') only_plus = 0;

    for (int w = 0; w < wi; w++)
        for (int i = 0; words[w][i]; i++) {
            unsigned char ch = words[w][i];
            if (!seen[ch]) { seen[ch] = 1; letters[nlet++] = ch; }
        }

    memset(is_leading, 0, sizeof(is_leading));
    for (int w = 0; w < wi; w++)
        if (strlen(words[w]) > 1)
            is_leading[(unsigned char)words[w][0]] = 1;

    maxlen = (int)strlen(words[nadd]);
}

long wval(const char *w) {
    long v = 0;
    for (int i = 0; w[i]; i++)
        v = v * 10 + val[(unsigned char)w[i]];
    return v;
}

void order_letters(void) {
    for (int i = 0; i < nlet; i++) {
        for (int j = i + 1; j < nlet; j++) {
            if (!is_leading[(unsigned char)letters[i]] && is_leading[(unsigned char)letters[j]]) {
                char t = letters[i];
                letters[i] = letters[j];
                letters[j] = t;
            }
        }
    }
}

void solve_col(int c, int carry);

void assign_and_check(int c, int carry_in, char *newl, int ncount, int idx) {
    if (found) return;

    if (idx == ncount) {
        long sum = carry_in;
        for (int w = 0; w < nadd; w++) {
            int len = (int)strlen(words[w]);
            if (len > c)
                sum += val[(unsigned char)words[w][len - 1 - c]];
        }
        int rlen = (int)strlen(words[nadd]);
        char rletter = (rlen > c) ? words[nadd][rlen - 1 - c] : 0;
        if (!rletter) return;

        if (sum % 10 != val[(unsigned char)rletter]) return;
        int carry_out = (int)(sum / 10);
        solve_col(c + 1, carry_out);
        return;
    }

    char l = newl[idx];
    for (int d = 0; d <= 9 && !found; d++) {
        if (used[d]) continue;
        if (d == 0 && is_leading[(unsigned char)l]) continue;
        used[d] = 1;
        val[(unsigned char)l] = d;
        assign_and_check(c, carry_in, newl, ncount, idx + 1);
        if (found) return;
        used[d] = 0;
        val[(unsigned char)l] = -1;
    }
}

void solve_col(int c, int carry) {
    if (found) return;

    if (c == maxlen) {
        if (carry == 0) found = 1;
        return;
    }

    char newl[12];
    int ncount = 0;
    int seen[256] = {0};

    for (int w = 0; w < nadd; w++) {
        int len = (int)strlen(words[w]);
        if (len > c) {
            unsigned char letter = words[w][len - 1 - c];
            if (val[letter] == -1 && !seen[letter]) {
                seen[letter] = 1;
                newl[ncount++] = letter;
            }
        }
    }

    int rlen = (int)strlen(words[nadd]);
    if (rlen > c) {
        unsigned char rletter = words[nadd][rlen - 1 - c];
        if (val[rletter] == -1 && !seen[rletter]) {
            seen[rletter] = 1;
            newl[ncount++] = rletter;
        }
    }

    for (int i = 0; i < ncount; i++) {
        for (int j = i + 1; j < ncount; j++) {
            if (!is_leading[(unsigned char)newl[i]] && is_leading[(unsigned char)newl[j]]) {
                char t = newl[i];
                newl[i] = newl[j];
                newl[j] = t;
            }
        }
    }

    assign_and_check(c, carry, newl, ncount, 0);
}

int checkeq(void) {
    long left = wval(words[0]);
    for (int i = 0; i < nadd - 1; i++) {
        long rhs = wval(words[i + 1]);
        char op = ops[i];
        if (op == '+') left += rhs;
        else if (op == '-') left -= rhs;
        else if (op == '*') left *= rhs;
        else if (op == '/') {
            if (rhs == 0 || left % rhs != 0) return 0;
            left /= rhs;
        }
    }
    return left == wval(words[nadd]);
}

void go(int idx) {
    if (found) return;

    if (idx == nlet) {
        if (checkeq()) found = 1;
        return;
    }

    char l = letters[idx];
    for (int d = 0; d <= 9 && !found; d++) {
        if (used[d]) continue;
        if (d == 0 && is_leading[(unsigned char)l]) continue;
        used[d] = 1;
        val[(unsigned char)l] = d;
        go(idx + 1);
        used[d] = 0;
    }
}

int solve(const char *in, char *out, size_t sz) {
    parse(in);
    if (nlet > 10) return 0;
    order_letters();

    memset(used, 0, sizeof(used));
    memset(val, -1, sizeof(val));
    found = 0;

    if (only_plus)
        solve_col(0, 0);
    else
        go(0);

    if (!found) return 0;

    char tmp[256] = {0};
    int off = 0;
    for (int w = 0; w < nadd; w++) {
        off += snprintf(tmp + off, sz - off, "%ld", wval(words[w]));
        if (w != nadd - 1)
            off += snprintf(tmp + off, sz - off, " %c ", ops[w]);
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

    int ok = solve(in, out, sizeof(out));

    if (ok)
        printf("Решение: %s\n", out);
    else
        printf("Решение не найдено.\n");

    clock_t t0 = clock();
    int reps = 0;
    double elapsed_ms = 0;
    do {
        solve(in, out, sizeof(out));
        reps++;
        elapsed_ms = (double)(clock() - t0) * 1000.0 / CLOCKS_PER_SEC;
    } while (elapsed_ms < 200 && reps < 2000000);

    double avg_ms = elapsed_ms / reps;
    printf("Время решения (среднее по %d повторам): %.5f мс\n", reps, avg_ms);

    return 0;
}
