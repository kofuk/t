#include "hoge.h"

struct _SampleHoge {
    GObject parent_instance;

    int count;
};

enum { PROP_0, PROP_COUNT, N_PROP };

G_DEFINE_TYPE(SampleHoge, sample_hoge, G_TYPE_OBJECT)

static GParamSpec *props[N_PROP] = {0};

static void sample_hoge_set_property(GObject *object, guint prop_id,
                                     const GValue *value, GParamSpec *pspec) {
    SampleHoge *self = SAMPLE_HOGE(object);

    g_print("Property set: %u\n", prop_id);

    switch (prop_id) {
    case PROP_COUNT:
        self->count = g_value_get_int(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void sample_hoge_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    SampleHoge *self = SAMPLE_HOGE(object);

    g_print("Property get: %u\n", prop_id);

    switch (prop_id) {
    case PROP_COUNT:
        g_value_set_int(value, self->count);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void sample_hoge_dispose(G_GNUC_UNUSED GObject *object) {
    g_print("Object disposed\n");
}

static void sample_hoge_init(SampleHoge *hoge) { hoge->count = 0; }

static void sample_hoge_class_init(SampleHogeClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = sample_hoge_set_property;
    object_class->get_property = sample_hoge_get_property;
    object_class->dispose = sample_hoge_dispose;

    props[PROP_COUNT] = g_param_spec_int("count", "Count", "Internal count", 0,
                                         G_MAXINT, 0, G_PARAM_READWRITE);
    g_object_class_install_properties(object_class, N_PROP, props);
}

SampleHoge *sample_hoge_new(void) {
    return g_object_new(SAMPLE_TYPE_HOGE, NULL);
}
