#include <glib.h>

#include "hoge.h"

int main(void) {
    SampleHoge *hoge = sample_hoge_new();
    g_object_set(hoge, "count", 5, NULL);
    g_object_unref(hoge);
}
