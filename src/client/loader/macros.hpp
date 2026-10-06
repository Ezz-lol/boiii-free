#ifndef DEFINE_COMPONENT_NAME
#ifdef NDEBUG
#define DEFINE_COMPONENT_NAME(name)
#else
#define DEFINE_COMPONENT_NAME(component_name)                                  \
  const std::string_view &name() override {                                    \
    static constexpr std::string_view name = component_name;                   \
    return name;                                                               \
  }
#endif
#endif
