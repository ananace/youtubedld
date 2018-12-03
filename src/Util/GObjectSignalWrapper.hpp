#pragma once

#include <string>

#include <glibmm.h>

namespace Glib
{
  template <typename T>
  static constexpr T wrap (T v, bool=true)
  {
    return v;
  }

  template <typename T>
  static constexpr T unwrap (T v, bool=true)
  {
    return v;
  }

  template<typename T>
  using unwrapped_t = decltype(unwrap(*((typename std::remove_reference<T>::type*)nullptr)));

  template<typename T>
  constexpr T return_helper()
  {
    typedef unwrapped_t<T> Ret;
    return Ret();
  }

  template<>
  constexpr void return_helper()
  {
    return void();
  }
}


template<typename>
class signal_callback;

template<typename Ret, typename ...T>
class signal_callback<Ret(T...)>
{
  template<typename ...Args>
  static auto callback(void* self, Args ...v)
  {
    using Glib::wrap;
    typedef sigc::slot< void, decltype(wrap(v))... > SlotType;

    void* data = std::get<sizeof...(Args)-1>(std::tuple<Args...>(v...));

    // Do not try to call a signal on a disassociated wrapper.
    if(dynamic_cast<Glib::Object*>(Glib::ObjectBase::_get_current_wrapper((GObject*) self)))
      {
	try
	  {
	    if(sigc::slot_base *const slot = Glib::SignalProxyNormal::data_to_slot(data))
	      {
		(*static_cast<SlotType*>(slot))(wrap(std::forward<Args>(v), true)...);
	      }
	  }
	catch(...)
	  {
	    Glib::exception_handlers_invoke();
	  }
      }

    return Glib::return_helper<Ret>();
  }

public:
  auto operator()(const std::string& signal_name, const Glib::RefPtr<Glib::Object>& obj)
  {
    using Glib::unwrap;
    static std::map<std::pair<GType, std::string>, Glib::SignalProxyInfo> signal_infos;

    auto key = std::make_pair(G_TYPE_FROM_INSTANCE (obj->gobj()), signal_name);
    if (signal_infos.find(key) == signal_infos.end())
      {
	signal_infos[key] = {
	  signal_name.c_str(),
	  (GCallback) &callback<Glib::unwrapped_t<T>..., void*>,
	  (GCallback) &callback<Glib::unwrapped_t<T>..., void*>
	};
      }

    return Glib::SignalProxy<Ret, T... >(obj.operator->(), &signal_infos[key]);
  }
};
