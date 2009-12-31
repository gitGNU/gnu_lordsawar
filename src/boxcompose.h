/* Defines Box::ucompose(fmt, arg...) for easy, i18n-friendly
 * composition of labels and images with Gtkmm >= 1.3.* (see www.gtkmm.org).
 * Uses Glib::ustring instead of std::string which doesn't work with
 * Gtkmm due to character encoding troubles with stringstreams.
 *
 * Version 1.0.4.
 *
 * Copyright (c) 2002, 03, 04 Ole Laursen <olau@hardworking.dk>.
 * modified from String::ucompose by Ole.
 * Changed by Ben Asselstine to create HBoxes instead.
 * Copyright (C) 2009 Ben Asselstine
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
 * 02110-1301, USA.
 */

//
// Basic usage is like
//
//   Box::ucompose("This is a %1x%2 matrix.", rows, cols);
//

#ifndef BOX_UCOMPOSE_H
#define BOX_UCOMPOSE_H

#include <glibmm/ustring.h>
#include <glibmm/convert.h>

#include <sstream>
#include <string>
#include <list>
#include <map>			// for multimap

namespace UBoxPrivate
{
  // the actual composition class - using String::ucompose is cleaner, so we
  // hide it here
  class Composition
  {
  public:
    // initialize and prepare format string on the form "text %1 text %2 etc."
    explicit Composition(std::string fmt);

    // supply an replacement argument starting from %1
    template <typename T>
    Composition &arg(const T &obj);

    // compose and return string
    Gtk::HBox *box() const;

  private:
    //Glib::ustring str() const;
    std::wostringstream os;
    int arg_no;

    // we store the output as a list - when the output string is requested, the
    // list is concatenated to a string; this way we can keep iterators into
    // the list instead of into a string where they're possibly invalidated
    // when inserting a specification string
    typedef std::list<std::string> output_list;
    output_list output;

    // the initial parse of the format string fills in the specification map
    // with positions for each of the various %?s
    typedef std::multimap<int, output_list::iterator> specification_map;
    specification_map specs;

    template <typename T>
    std::string stringify(T obj);
  };

  // helper for converting spec string numbers
  inline int char_to_int(char c)
  {
    switch (c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    default: return -1000;
    }
  }

  inline bool is_number(int n)
  {
    switch (n) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return true;
    
    default:
      return false;
    }
  }

  template <typename T>
  inline std::string Composition::stringify(T obj)
  {
    os << obj;

    std::wstring str = os.str();
    
    return Glib::convert(std::string(reinterpret_cast<const char *>(str.data()),
				     str.size() * sizeof(wchar_t)),
			 "UTF-8", "WCHAR_T");
  }

  // specialisations for the common string types
  template <>
  inline std::string
  Composition::stringify<std::string>(std::string obj)
  {
    return obj;
  }
  
  template <>
  inline std::string
  Composition::stringify<Glib::ustring>(Glib::ustring obj)
  {
    return obj;
  }
  
  template <>
  inline std::string
  Composition::stringify<const char *>(const char *obj)
  {
    return obj;
  }
  
  template <>
  inline std::string
  Composition::stringify<Glib::RefPtr<Gdk::Pixbuf> >(Glib::RefPtr<Gdk::Pixbuf> obj)
  {
    char buf[32];
    snprintf(buf, sizeof(buf), "Gdk::Pixbuf %p", obj->gobj());
    return buf;
  }
  
  // implementation of class Composition
  template <typename T>
  inline Composition &Composition::arg(const T &obj)
  {
    Glib::ustring rep = stringify(obj);
    
    if (!rep.empty()) {		// manipulators don't produce output
      for (specification_map::const_iterator i = specs.lower_bound(arg_no),
	     end = specs.upper_bound(arg_no); i != end; ++i) {
	output_list::iterator pos = i->second;
	++pos;
      
	output.insert(pos, rep);
      }
    
      os.str(std::wstring());
      //os.clear();
      ++arg_no;
    }
  
    return *this;
  }

  inline Composition::Composition(std::string fmt)
    : arg_no(1)
  {
//#if __GNUC__ >= 3
    //os.imbue(std::locale("")); // use the user's locale for the stream
//#endif
    std::string::size_type b = 0, i = 0;
  
    // fill in output with the strings between the %1 %2 %3 etc. and
    // fill in specs with the positions
    while (i < fmt.length()) {
      if (fmt[i] == '%' && i + 1 < fmt.length()) {
	if (fmt[i + 1] == '%') { // catch %%
	  fmt.replace(i, 2, "%");
	  ++i;
	}
	else if (is_number(fmt[i + 1])) { // aha! a spec!
	  // save string
	  output.push_back(fmt.substr(b, i - b));
	
	  int n = 1;		// number of digits
	  int spec_no = 0;

	  do {
	    spec_no += char_to_int(fmt[i + n]);
	    spec_no *= 10;
	    ++n;
	  } while (i + n < fmt.length() && is_number(fmt[i + n]));

	  spec_no /= 10;
	  output_list::iterator pos = output.end();
	  --pos;		// safe since we have just inserted a string
	
	  specs.insert(specification_map::value_type(spec_no, pos));
	
	  // jump over spec string
	  i += n;
	  b = i;
	}
	else
	  ++i;
      }
      else
	++i;
    }
  
    if (i - b > 0)		// add the rest of the string
      output.push_back(fmt.substr(b, i - b));
  }

  //inline Glib::ustring Composition::str() const
  //{
    // assemble string
    //std::string str;
  
    //for (output_list::const_iterator i = output.begin(), end = output.end();
	 //i != end; ++i)
      //str += *i;
  
    //return str;
  //}
  inline Gtk::HBox* Composition::box() const
  {
    Gtk::HBox *hbox = new Gtk::HBox();
    for (output_list::const_iterator i = output.begin(), end = output.end();
	 i != end; ++i)
      {
        Gtk::Label *label = new Gtk::Label(*i);

        void *ptr = NULL;
        int retval = sscanf (label->get_text().c_str(), "Gdk::Pixbuf %p", &ptr);
        if (retval != 1)
          hbox->pack_start(*manage(label), Gtk::PACK_SHRINK, 0);
        else
          {
            Glib::RefPtr<Gdk::Pixbuf> pic = Glib::wrap((GdkPixbuf*)ptr, true);
            Gtk::Image *image = new Gtk::Image();
            image->property_pixbuf() = pic;
            hbox->pack_start(*manage(image), Gtk::PACK_SHRINK, 0);
          }
      }
      return hbox;
  }
}


namespace Box
{
  // a series of functions which accept a format string on the form "text %1
  // more %2 less %3" and a number of templated parameters and spits out the
  // composited string
  template <typename T1>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt, const T1 &o1)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1);
    return c.box();
  }

  template <typename T1, typename T2>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2);
    return c.box();
  }

  template <typename T1, typename T2, typename T3>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3);
    return c.box();
  }

  template <typename T1, typename T2, typename T3, typename T4>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4);
    return c.box();
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4).arg(o5);
    return c.box();
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4).arg(o5).arg(o6);
    return c.box();
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6, typename T7>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6,
				const T7 &o7)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4).arg(o5).arg(o6).arg(o7);
    return c.box();
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6, typename T7, typename T8>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6,
				const T7 &o7, const T8 &o8)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4).arg(o5).arg(o6).arg(o7).arg(o8);
    return c.box();
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6, typename T7, typename T8, typename T9>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6,
				const T7 &o7, const T8 &o8, const T9 &o9)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4).arg(o5).arg(o6).arg(o7).arg(o8).arg(o9);
    return c.box();
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6, typename T7, typename T8, typename T9, typename T10>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6,
				const T7 &o7, const T8 &o8, const T9 &o9,
				const T10 &o10)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4).arg(o5).arg(o6).arg(o7).arg(o8).arg(o9)
      .arg(o10);
    return c.box();
  }
  
  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6, typename T7, typename T8, typename T9, typename T10,
	    typename T11>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6,
				const T7 &o7, const T8 &o8, const T9 &o9,
				const T10 &o10, const T11 &o11)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4).arg(o5).arg(o6).arg(o7).arg(o8).arg(o9)
      .arg(o10).arg(o11);
    return c.box();
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6, typename T7, typename T8, typename T9, typename T10,
	    typename T11, typename T12>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6,
				const T7 &o7, const T8 &o8, const T9 &o9,
				const T10 &o10, const T11 &o11, const T12 &o12)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4).arg(o5).arg(o6).arg(o7).arg(o8).arg(o9)
      .arg(o10).arg(o11).arg(o12);
    return c.box();
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6, typename T7, typename T8, typename T9, typename T10,
	    typename T11, typename T12, typename T13>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6,
				const T7 &o7, const T8 &o8, const T9 &o9,
				const T10 &o10, const T11 &o11, const T12 &o12,
				const T13 &o13)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4).arg(o5).arg(o6).arg(o7).arg(o8).arg(o9)
      .arg(o10).arg(o11).arg(o12).arg(o13);
    return c.box();
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6, typename T7, typename T8, typename T9, typename T10,
	    typename T11, typename T12, typename T13, typename T14>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6,
				const T7 &o7, const T8 &o8, const T9 &o9,
				const T10 &o10, const T11 &o11, const T12 &o12,
				const T13 &o13, const T14 &o14)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4).arg(o5).arg(o6).arg(o7).arg(o8).arg(o9)
      .arg(o10).arg(o11).arg(o12).arg(o13).arg(o14);
    return c.box();
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6, typename T7, typename T8, typename T9, typename T10,
	    typename T11, typename T12, typename T13, typename T14,
	    typename T15>
  inline Gtk::HBox* ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6,
				const T7 &o7, const T8 &o8, const T9 &o9,
				const T10 &o10, const T11 &o11, const T12 &o12,
				const T13 &o13, const T14 &o14, const T15 &o15)
  {
    UBoxPrivate::Composition c(fmt);
    c.arg(o1).arg(o2).arg(o3).arg(o4).arg(o5).arg(o6).arg(o7).arg(o8).arg(o9)
      .arg(o10).arg(o11).arg(o12).arg(o13).arg(o14).arg(o15);
    return c.box();
  }
}


#endif // BOX_UCOMPOSE_HPP
