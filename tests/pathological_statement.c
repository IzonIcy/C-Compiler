/* Regression test: an expression statement whose parse fails without
 * consuming tokens (here: starts with ')') used to return a non-NULL
 * empty node, making the compound-statement loop spin forever while
 * emitting one diagnostic per iteration. The parser now returns NULL
 * and the statement loop synchronizes past the garbage.
 */
int main(void) {
    ))))))))
}
