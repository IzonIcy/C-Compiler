/* Regression test: '?' with no parseable condition used to dereference
 * the NULL condition node (SEGV). The parser now bails out cleanly.
 */
int main(void) {
    ? 3;
}
