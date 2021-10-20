#include <vterm.h>

int main(void) {
    VTerm *vt = vterm_new(80, 24);
    vterm_input_write(vt, "hogehoge", 8);
    vterm_free(vt);
}
