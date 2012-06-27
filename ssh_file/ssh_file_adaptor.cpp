//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/adaptors/adaptor.hpp>

#include "ssh_file_adaptor_file.hpp"
#include "ssh_file_adaptor_dir.hpp"

namespace ssh_filesystem_adaptor
{
  SAGA_ADAPTOR_REGISTER (filesystem_adaptor);

  ///////////////////////////////////////////////////////////////////////////////
  //
  // adaptor registration
  //
  saga::impl::adaptor_selector::adaptor_info_list_type
    filesystem_adaptor::adaptor_register (saga::impl::session * s)
  {
    // list of implemented cpi's
    saga::impl::adaptor_selector::adaptor_info_list_type infos;
    preference_type prefs; 

    // tell the engine to only call this ssh adaptor if an ssh context is
    // available
    prefs.set (saga::attributes::context_type, "ssh");

    file_cpi_impl::register_cpi (infos, prefs, adaptor_uuid_);
    dir_cpi_impl::register_cpi  (infos, prefs, adaptor_uuid_);

    return infos;
  }

  filesystem_adaptor::filesystem_adaptor (void)
  {
    // std::cout << " mounted_        ctor   :  " << this             << std::endl;
    // std::cout << " mounted_.size() ctor   :  " << mounted_.size () << std::endl;
    // std::cout << " tested__.size() ctor   :  " << tested__.size () << std::endl;
  }


  // on destruction, umount all sshfs file systems - if keepalive is not set
  filesystem_adaptor::~filesystem_adaptor (void)
  {
    // release the sshfs shared_ptr.  On sshfs destruction, the file system
    // is umounted.
    //
    // Well, the above was theory.  In practice, the shared pointer do not seem
    // to get released even if we call mounted_.clear() here - other copies are
    // somewhere lingering.  So, we call umount directly (if keepalive is not
    // set), and rely on the session management to only call us when no
    // operations on the mounted filesystems are running anymore.
    
    // std::cout << " mounted_        dtor   :  " << this             << std::endl;
    // std::cout << " mounted_.size() dtor   :  " << mounted_.size () << std::endl;
    // std::cout << " tested__.size() dtor   :  " << tested__.size () << std::endl;

    if ( ini_.find ("keepalive") != ini_.end () &&
         ( ini_["keepalive"] == "yes"  ||
           ini_["keepalive"] == "true" )        )
    {
      // std::cout << " === keep stuff mounted" << std::endl;
      if ( mounted_.size () > 0 )
      {
        SAGA_LOG_INFO ("keeping sshfs filesystems mounted");
      }
    }
    else
    {
      std::map <std::string, TR1::shared_ptr <sshfs> > :: iterator begin = mounted_.begin ();
      std::map <std::string, TR1::shared_ptr <sshfs> > :: iterator end   = mounted_.end   ();
      std::map <std::string, TR1::shared_ptr <sshfs> > :: iterator it;

      for ( it = begin; it != end; it++ )
      {
        // std::cout << " umount for " << it->first << std::endl;
        (*it).second->umount ();
      }
    }
  }


  // cache ini section on init
  bool filesystem_adaptor::init (saga::impl::session  * s,
                           saga::ini::ini const & glob_ini,
                           saga::ini::ini const & adap_ini)
  {
    if ( adap_ini.has_section ("preferences") )
    {
      ini_ = adap_ini.get_section ("preferences").get_entries ();
    }
      
    return true;
  } 

  
  std::string filesystem_adaptor::get_sshfs_id (const saga::url & u)
  {
    std::string host = u.get_host     ();
    int         port = u.get_port     ();
    std::string user = u.get_username ();

    std::stringstream ss;

    if ( ! user.empty () )   ss << user << "@" ;
    if ( ! host.empty () )   ss << host;
    if ( port == -1      )   ss << ":"  << 22  ;
    else                     ss << ":"  << port;

    return ss.str ();
  }


  TR1::shared_ptr <sshfs> filesystem_adaptor::get_sshfs (const saga::session & s, 
                                                         const saga::url     & u)
  {
    // only mount anything if the URL is ssh (or any) based
    if ( u.get_scheme () != "ssh" &&
         u.get_scheme () != "any" )
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("Cannot mount sshfs for non-ssh urls", 
                                     saga::BadParameter);
    }

    return mount_sshfs (s, u);
  }

  sshfs_t filesystem_adaptor::mount_sshfs (const saga::session & s, const saga::url & u )
  {
    saga::url   tgt = u;

    // if url is any:// based, convert to ssh:// as expected by sshfs
    if ( tgt.get_scheme () == "any" )
    {
      tgt.set_scheme ("ssh");
    }

    std::string id  = get_sshfs_id (tgt);

    
    // std::cout << " mounted_        mount  :  " << this             << std::endl;
    // std::cout << " mounted_.size() mount 1:  " << mounted_.size () << std::endl;
    // std::cout << " tested__.size() mount 1:  " << tested__.size () << std::endl;

    // check if we have that mounted already
    if ( mounted_.find (id) != mounted_.end () )
    {
      // make sure the fs is mounted
      mounted_[id]->mount ();

      return mounted_[id];
    }

    // is not mounted, yet - try to mount it, store a new shared pointer, and
    // return it.
    SAGA_LOG_ALWAYS ("to mount new sshfs");
    SAGA_LOG_ALWAYS (id.c_str ());
    SAGA_LOG_ALWAYS (tgt.get_string ().c_str ());

    TR1::shared_ptr <sshfs> ptr;
    try 
    {
      ptr.reset (new sshfs (ini_, s, tgt));
    }
    catch ( const saga::exception & e )
    {
      SAGA_LOG_ERROR (e.what ());
      std::stringstream ss;
      ss << "Could not mount sshfs for " << tgt << " : \n  " << e.what () <<
        std::endl;
      SAGA_ADAPTOR_THROW_NO_CONTEXT (ss.str ().c_str (), saga::BadParameter);
    }

    if ( ! ptr )
    {
      // cannot mount the fs for some reason - throw a BadParameter
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("sshfs mount failed for unknown reason",
                                     saga::NoSuccess);
    }

    // std::cout << " mounted_.size() mount 2:  " << mounted_.size () << std::endl;
    // std::cout << " tested__.size() mount 2:  " << tested__.size () << std::endl;

    // got the fs mounted - register it, and return the ptr
    mounted_[id] = ptr;
    tested__[id] = 1;
    SAGA_LOG_ALWAYS ("register mounted sshfs");
    SAGA_LOG_ALWAYS (id.c_str ());

    // std::cout << " === register mounted sshfs: " << id << std::endl;
    // std::cout << " mounted_.size() mount 3:  " << mounted_.size () << std::endl;
    // std::cout << " tested__.size() mount 3:  " << tested__.size () << std::endl;

    return ptr;
  }


  // returns a clone of the given session, but without any ssh context.  That
  // way we ensure that future calls in that stripped session will not arrive 
  // in this adaptor again, ever (see also the comment on adaptor preferences 
  // in adaptor_register().
  saga::session filesystem_adaptor::strip_session (const saga::session & s)
  {
    saga::session ret = static_cast <saga::session> (s.clone ());

    std::vector <saga::context> cs = ret.list_contexts ();

    for ( unsigned int i = 0; i < cs.size (); i++ )
    {
      if ( cs[i].get_attribute (saga::attributes::context_type) == "ssh" )
      {
        ret.remove_context (cs[i]);
      }
    }

    return ret;
  }


  bool filesystem_adaptor::is_absolute (const saga::url & u)
  {
    if ( u.get_scheme   ().empty ()  &&
         u.get_host     ().empty ()  &&
         u.get_username ().empty ()  &&
         u.get_password ().empty ()  &&
         u.get_port     ()    == -1  &&
         u.get_path     ()[0] != '/' )
    {
      return false;
    }
    
    return true;
  }


  saga::url filesystem_adaptor::make_absolute (const saga::url & base, 
                                               const saga::url & u)
  {
    if ( is_absolute (u) )
    {
      return u;
    }

    // relative 
    saga::url ret (base); // copy scheme, host, etc
    ret.set_path  (base.get_path () + "/" + u.get_path ());

    return ret;
  }

  // search all known sshfs's tto translate the URL  If none scceeds, try to
  // create a new sshfs mount, and work on it
  saga::url filesystem_adaptor::translate (const saga::session & s, 
                                           const saga::url     & u)
  {
    saga::url ret;

    try {
      // FIXME: we would actually need the cpi's session here...
      sshfs_t sshfs = get_sshfs (s, u);

      ret = sshfs->translate (u);
    }
    catch ( const saga::exception & e )
    {
      std::stringstream ss;
      ss << "Cannot handle URL 1" << u 
         << ": cannot mount/translate into local file system : \n"
         << e.what () << "\n";
      SAGA_ADAPTOR_THROW_NO_CONTEXT (ss.str (), saga::NoSuccess);
    }


    if ( ret == u )
    {
      std::stringstream ss;
      ss << "Cannot handle URL 2" << u << ": cannot mount/translate into local file system";
      SAGA_ADAPTOR_THROW_NO_CONTEXT (ss.str (), saga::NoSuccess);
    }

    return ret;
  }


  // same as translate, but simply returns the original url on failure
  // cveate a new sshfs mount, and work on it
  saga::url filesystem_adaptor::try_translate (const saga::session & s,
                                               const saga::url     & u)
  {
    try 
    {
      sshfs_t sshfs = get_sshfs (s, u);
      return  sshfs->translate  (u);
    }
    catch ( const saga::exception & e )
    {
      std::stringstream ss;
      ss << "Cannot handle URL 1" << u 
         << ": cannot mount/translate into local file system : \n"
         << e.what () << "\n";
      // SAGA_ADAPTOR_THROW_NO_CONTEXT (ss.str (), saga::NoSuccess);
    }

    return u;
  }


  // search all known sshfs's tto translate the URL *back*. If none succeeds,
  // try to create a new sshfs mount, and work on it
  //
  // not that in many cases, non being to translated-back is acceptable - some
  // file systems are wierd, and even non-wierd ones may return a relative link
  // target (i.e., no back translation, which requires an absolute path).  So
  // callers should try to handle exceptions gracefully.

  saga::url filesystem_adaptor::translate_back (const saga::session & s, 
                                                const saga::url     & u)
  {
    std::string err;

    // std::cout << " mounted_.size(): trans 1 " << mounted_.size () << std::endl;
    std::map <std::string, TR1::shared_ptr <sshfs> > :: iterator begin = mounted_.begin ();
    std::map <std::string, TR1::shared_ptr <sshfs> > :: iterator end   = mounted_.end   ();
    std::map <std::string, TR1::shared_ptr <sshfs> > :: iterator it;

    for ( it = begin; it != end; it++ )
    {
      try
      {
        // std::cout << " mounted_.size(): trans 2 " << mounted_.size () << std::endl;
        return (*it).second->translate_back (u);
      }
      catch ( const saga::exception & e )
      {
        // try next one
        // std::cout << " mounted_.size(): trans 3 " << mounted_.size () << std::endl;
        err += e.get_message ();

        continue;
      }
    }

    // std::cout << " mounted_.size(): trans 4 " << mounted_.size () << std::endl;
    // could not translate back it seems - throw
    SAGA_ADAPTOR_THROW_NO_CONTEXT ((std::string ("Cannot handle URL 3: ") + err).c_str (), 
                                   saga::BadParameter);

    // keep compiler happy
    return "";
  }

  // as translate_back, but simply returns the original url on failure
  saga::url filesystem_adaptor::try_translate_back (const saga::session & s, 
                                                    const saga::url     & u)
  {
    try
    {
      return translate_back (s, u);
    }
    catch ( const saga::exception & /*e*/ )
    {
      return u;
    }
  }

  void filesystem_adaptor::dump_session (const std::string   & msg, 
                                         const saga::session & s)
  {
    std::cout << " ---------------------------- " << std::endl;
    std::cout << " -- " << msg << " -- "          << std::endl;
    std::cout << " ---------------------------- " << std::endl;

    std::vector <saga::context> cs = s.list_contexts ();
    std::cout << " found " << cs.size () << " contexts " << std::endl;

    for ( unsigned int i = 0; i < cs.size (); i++ )
    {
      std::cout << "  " << i << ": " << cs[i].get_attribute ("Type") << std::endl;
    }

    std::cout << " ---------------------------- " << std::endl;
  }

} // namespace

