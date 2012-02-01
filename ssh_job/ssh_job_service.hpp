//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_SSH_JOB_SERVICE_HPP
#define ADAPTORS_SSH_JOB_SERVICE_HPP

// stl includes
#include <string>
#include <iosfwd>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/engine/proxy.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/adaptor_data.hpp>

// saga package includes
#include <saga/impl/packages/job/job_service_cpi.hpp>

// adaptor includes
#include "ssh_job_adaptor.hpp"


////////////////////////////////////////////////////////////////////////
namespace ssh_job
{
  class job_service_cpi_impl 
    : public saga::adaptors::v1_0::job_service_cpi <job_service_cpi_impl>
  {
    private:
      typedef saga::adaptors::v1_0::job_service_cpi <job_service_cpi_impl> 
              base_cpi;

      // adaptor data
      typedef saga::adaptors::adaptor_data <adaptor> adaptor_data;

      // state
      saga::url                            rm_;
      saga::session                        s_;
      saga::context                        ctx_;

      saga::job::service                   js_;

      std::string                          host_;
      std::string                          path_;
      std::string                          key_;
      int                                  port_;
      std::string                          user_;
      std::string                          local_user_;

      std::string                          gsissh_bin_;
      std::vector <std::string>            gsissh_opt_;

      std::string                          ssh_bin_;
      std::vector <std::string>            ssh_opt_;

      std::string                          parent_id_;    // id of self
      std::string                          loc_ssh_key_pub_;  // local ssh key
      std::string                          loc_ssh_key_priv_; // local ssh key
      std::string                          rem_ssh_key_pub_;  // deployed ssh key
      std::string                          rem_ssh_key_priv_; // deployed ssh key


      std::map <std::string, std::string>  ini_;
      std::vector <std::string>            env_;

      bool                                 use_gsi_;

      void dump_context_ (saga::context c);
      void check_ini_    (void);


    public:
      // constructor of the job_service cpi
      job_service_cpi_impl  (proxy                           * p, 
                             cpi_info const                  & info,
                             saga::ini::ini const            & glob_ini, 
                             saga::ini::ini const            & adap_ini,
                             TR1::shared_ptr <saga::adaptor>   adaptor);

      // destructor of the job_service cpi
      ~job_service_cpi_impl (void);

      // CPI functions
      void sync_create_job (saga::job::job            & ret, 
                            saga::job::description      jd);
      void sync_run_job    (saga::job::job            & ret, 
                            std::string                 cmd, 
                            std::string                 host, 
                            saga::job::ostream        & in, 
                            saga::job::istream        & out, 
                            saga::job::istream        & err);
      void sync_list       (std::vector <std::string> & ret);
      void sync_get_job    (saga::job::job            & ret,
                            std::string                 jobid);
      void sync_get_self   (saga::job::self           & ret);

  };  // class job_service_cpi_impl

} // namespace ssh_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_SSH_JOB_SERVICE_HPP

