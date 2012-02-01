//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ssh_context_adaptor.hpp"

#include <saga/saga.hpp>

#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/attribute.hpp>

#include <sys/stat.h>
#include <pwd.h>



using namespace ssh_context_adaptor;

#define ADAPTORS_SSH_CONTEXT_TYPE "ssh"

SAGA_ADAPTOR_REGISTER (context_adaptor);

///////////////////////////////////////////////////////////////////////////////
//  constructor
context_cpi_impl::context_cpi_impl (proxy                * p, 
                                    cpi_info       const & info,
                                    saga::ini::ini const & glob_ini, 
                                    saga::ini::ini const & adap_ini,
                                    TR1::shared_ptr <saga::adaptor> adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
{
  saga::adaptors::attribute attr (this);

  if ( attr.attribute_exists (saga::attributes::context_type) )
  {
    if ( ADAPTORS_SSH_CONTEXT_TYPE != 
         attr.get_attribute (saga::attributes::context_type) )
    {
      SAGA_OSSTREAM strm;
      strm << "Can't handle context types others than '"
              ADAPTORS_SSH_CONTEXT_TYPE "' "
           << "(got: " << attr.get_attribute(saga::attributes::context_type) 
           << ")";
      SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//  destructor
context_cpi_impl::~context_cpi_impl (void)
{
}

///////////////////////////////////////////////////////////////////////////////
//
void context_cpi_impl::sync_set_defaults (saga::impl::void_t &)
{    
  saga::adaptors::attribute attr (this);

  if ( attr.attribute_exists (saga::attributes::context_type) )
  {
    if ( ADAPTORS_SSH_CONTEXT_TYPE != 
         attr.get_attribute (saga::attributes::context_type) )
    {
      SAGA_OSSTREAM strm;
      strm << "Can't handle context types others than '" 
               ADAPTORS_SSH_CONTEXT_TYPE "' "
           << "(got: " << attr.get_attribute(saga::attributes::context_type) 
           << ")";
      SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
    }

    if ( "" == attr.get_attribute (saga::attributes::context_type) )
    {
      SAGA_ADAPTOR_THROW ("Context Type cannot have an empty value", 
                          saga::IncorrectState);
    }

    cert_info_t ci;

    if ( attr.attribute_exists (saga::attributes::context_userkey) ) 
    {
      // this call looks for a valid proxy file in the location described by
      // saga::attributes::context_userkey
      ci = get_cert_info (attr.get_attribute (saga::attributes::context_userkey));
    }
    else
    {
      // this call looks for a valid cert in a default location
      ci = get_cert_info ();    
    }

    if ( true == ci.success )
    {
      // found a valid cert, copy information over, if not set previously
      if (     ! attr.attribute_exists (saga::attributes::context_userkey) ||
           "" == attr.get_attribute    (saga::attributes::context_userkey) )
      {
        attr.set_attribute (saga::attributes::context_userkey, ci.private_key);
      }

      if (     ! attr.attribute_exists (saga::attributes::context_usercert) ||
           "" == attr.get_attribute    (saga::attributes::context_usercert) )
      {
        attr.set_attribute (saga::attributes::context_usercert, ci.public_key);
      }

      if (     ! attr.attribute_exists (saga::attributes::context_userid) ||
           "" == attr.get_attribute    (saga::attributes::context_userid) )
      {
        attr.set_attribute (saga::attributes::context_userid, ci.userid  );
      }
    }
    else
    {
      SAGA_ADAPTOR_THROW (ci.errormessage, saga::NoSuccess);
    }
  }
}
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
saga::impl::adaptor_selector::adaptor_info_list_type
context_adaptor::adaptor_register (saga::impl::session * s)
{
  // list of implemented cpi's
  saga::impl::adaptor_selector::adaptor_info_list_type infos;
  preference_type prefs; 

  context_cpi_impl::register_cpi (infos, prefs, adaptor_uuid_);

  // create a default security context if this is a default session
  if ( s->is_default_session () )
  {
    std::vector <std::pair <std::string, std::string> > entries;

    std::pair <std::string, std::string> entry (saga::attributes::context_type, 
                                                "ssh");
    entries.push_back (entry);

    s->add_proto_context (entries);

  }

  return infos;
}
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// init a cert, either from a given path, or from a default location
//
context_cpi_impl::cert_info_t context_cpi_impl::get_cert_info (std::string key_path)
{
  std::string key_path_pub;
  std::string key_user;

  cert_info_t ci;

  ci.success     = true;
  ci.private_key = key_path;


  // try to find fallbacks
  if ( ci.private_key == "" )
  {
    // if SAGA_SSH_KEY is set in the environment, we use that as default
    const char * saga_ssh_key = ::getenv ("SAGA_SSH_KEY");

    if ( NULL != saga_ssh_key )
    {
      ci.private_key = saga_ssh_key;

      // is the public key location also given?
      const char * saga_ssh_pub = ::getenv ("SAGA_SSH_PUB");

      if ( NULL != saga_ssh_pub )
      {
        key_path_pub = saga_ssh_pub;
      }

      // is the public key location also given?
      const char * saga_ssh_user = ::getenv ("SAGA_SSH_USER");

      if ( NULL != saga_ssh_user )
      {
        key_user = saga_ssh_user;
      }
    }
  }


  // anther fallback is ~/.{ssh,ssh2}/id_[dr]sa
  if ( ci.private_key == "" )
  {
    const char * home = ::getenv ("HOME");

    if ( home == NULL )
    {
      ci.errormessage = "Cannot determine home directory, i.e. default ssh key location";
      ci.success      = false;
      return ci;
    }


    saga::filesystem::directory d (std::string ("file://localhost/") + home + "/");

    std::string dot_dir; 

    if ( d.exists (".ssh") &&
         d.is_dir (".ssh") )
    {
      dot_dir = ".ssh";
    }
    else
    if ( d.exists (".ssh2") &&
         d.is_dir (".ssh2") )
    {
      dot_dir = ".ssh2";
    }
    else
    {
      ci.errormessage = "Cannot access private ssh key - not ~/.ssh[2]";
      ci.success      = false;
      return ci;
    }

    if ( d.exists  (dot_dir + "/id_rsa") &&
         d.is_file (dot_dir + "/id_rsa") )
    {
      ci.private_key = d.get_url ().get_path () + "/" + dot_dir + "/id_rsa"; 
    }
    else
    if ( d.exists  (dot_dir + "/id_dsa") &&
         d.is_file (dot_dir + "/id_dsa") )
    {
      ci.private_key = d.get_url ().get_path () + "/" + dot_dir + "/id_dsa"; 
    }
    else
    {
      ci.errormessage = "Cannot access private ssh key";
      ci.success      = false;
      return ci;
    }
  }




  // default for public key
  if ( key_path_pub   == "" &&
       ci.private_key != "" )
  {
    ci.public_key = ci.private_key + ".pub";
  }


  // default for key_user
  if ( key_user == "" )
  {
    key_user = ::getpwuid (::getuid ())->pw_name;
  }

  ci.userid  = key_user;



  // check if given private key, and related public-key, exist
  // FIXME: use saga::filesystem!
  struct stat buf;

  if ( 0 != ::stat (ci.private_key.c_str (), &buf) )
  {
    ci.errormessage = "Cannot access private ssh key";
    ci.success      = false;
    return ci;
  }

  if ( 0 != ::stat (ci.public_key.c_str (), &buf) )
  {
    // FIXME: this should be a warning, not an error
    // ci.errormessage  = "Cannot access public ssh key: ";
    // ci.errormessage += ci.public_key;
    // ci.success       = false;
    // return ci;
  }
  
  return ci;
}
//
///////////////////////////////////////////////////////////////////////////////

