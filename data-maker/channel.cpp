#include <bits/stdc++.h>
#include <algorithm>

using namespace std;

int main() {
    freopen("channel.txt", "r", stdin);
    freopen("channel.out", "w", stdout);
    
    int n;
    scanf("%d", &n);
    for (int i = 1; i <= n; ++ i) {
        int s, t, channel, cost;
        /*
        scanf("%d%d%d", &s, &t, &cost);
        for (int j = 0; j < 3 ; ++ j) {
            printf("if (src == %d && dst == %d && channel == %d)\n", s, t, j);
            printf("        return %d;\n", cost * (j + 1) );
            swap(s, t);
            printf("if (src == %d && dst == %d && channel == %d)\n", s, t, j);
            printf("        return %d;\n", cost * (j + 1) );
            swap(s, t);
        }
        */
        scanf("%d%d%d%d", &s, &t, &channel, &cost);
        printf("if (src == %d && dst == %d && channel == %d)\n", s, t, channel);
        printf("        return %d;\n", cost);
        swap(s, t);
        printf("if (src == %d && dst == %d && channel == %d)\n", s, t, channel);
        printf("        return %d;\n", cost);
        swap(s, t);
    }
    return 0;
}
