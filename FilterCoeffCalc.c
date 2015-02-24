/*
                                         ________
    Recursive multiplication in the form ||(1-Ri) by Pieter Suurmond, 2004.
*/

#include <stdlib.h>
#include <stdio.h>

int mul(int n)                          /* n is the number of roots. */
{
    int*    idx;
    int     i, j, a;
    
    idx = malloc(n * sizeof(int));
    if (!idx)
        { printf("Not enough memory!\n"); return 1; }
    printf("Number of roots = %d:", n);
    for (i = 0; i <= n; i++)
        {
        printf("\n coeff%d =", i);
        if (i)
            {
            a = 0; 
            idx[a] = 0;
            do  {
                for (j = a + 1; j < i; j++)   /* Init following indices. */
                    idx[j] = 1 + idx[j - 1];
                a = i-1;                      /* Select rightmost index. */
                if (i & 1) printf(" - ");
                else       printf(" + ");
                for (j = 0; j < i; j++)
                    {
                    if (j) printf("*");
                    printf("R%d", idx[j]);
                    }
                while ((a>=0) && (++idx[a] + (i-a) > n))   /* Inc index. */
                    a--;          /* Select preceding index on overflow. */
                } while (a >= 0);
            }
        else
            printf(" 1");
        }
    printf("\n\n");
    free(idx);
    return 0;
}

int main()
{
    int n, e = 0;
    
    for (n = 1; (!e) && (n < 8); n++)
        e = mul(n);
    return e;
}
