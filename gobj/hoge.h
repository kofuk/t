#ifndef HOGE_H
#define HOGE_H

#include <glib-object.h>

typedef struct _SampleHoge SampleHoge;

#define SAMPLE_TYPE_HOGE sample_hoge_get_type()
G_DECLARE_FINAL_TYPE(SampleHoge, sample_hoge, SAMPLE, HOGE, GObject)

SampleHoge *sample_hoge_new(void);

#endif
