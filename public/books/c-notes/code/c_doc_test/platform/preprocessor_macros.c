#define JOIN_IMPL(left, right) left##right
#define JOIN(left, right) JOIN_IMPL(left, right)
#define STRINGIFY_IMPL(value) #value
#define STRINGIFY(value) STRINGIFY_IMPL(value)
#define PREFIX_VALUE 42

#ifndef DOC_FEATURE
#error "DOC_FEATURE must be supplied by the checker"
#endif

#if DOC_FEATURE != 21
#error "unexpected DOC_FEATURE"
#endif

int main(void) {
    const char *feature_text = STRINGIFY(DOC_FEATURE);
    return JOIN(PREFIX_, VALUE) == 42 &&
                   feature_text[0] == '2' && feature_text[1] == '1' &&
                   feature_text[2] == '\0'
               ? 0
               : 1;
}
