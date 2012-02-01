//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_SSH_JOB_HPP
#define ADAPTORS_SSH_JOB_HPP

// stl includes
#include <string>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/engine/proxy.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/adaptor_data.hpp>

// job package includes
#include <saga/impl/packages/job/job_cpi.hpp>

// adaptor includes
#include "ssh_job_adaptor.hpp"

// #include "ssh_suite.hpp"

////////////////////////////////////////////////////////////////////////
namespace ssh_job
{
  class job_cpi_impl 
    : public saga::adaptors::v1_0::job_cpi <job_cpi_impl>
  {
    private:
      typedef saga::adaptors::v1_0::job_cpi <job_cpi_impl> base_cpi;

      // adaptor data
      typedef saga::adaptors::adaptor_data <adaptor> adaptor_data;

      /// typedef saga::adaptors::adaptor_suite_data <ssh::suite> suite_data;

      // state
      saga::url                            rm_;
      saga::context                        ctx_;
      
      TR1::shared_ptr <saga::job::service> js_;

      std::string                          host_;
      std::string                          user_;
      std::string                          key_;
      std::string                          path_;
      int                                  port_;

      std::string                          gsissh_bin_;
      std::vector <std::string>            gsissh_opt_;

      std::string                          ssh_bin_;
      std::vector <std::string>            ssh_opt_;

      std::vector <std::string>            ssh_args_;

      std::map <std::string, std::string>  ini_;

      saga::job::job                       j_;        // forward to default job

      std::string                          id_;       // cached job id
      std::string                          old_exe_;  // original executable
      saga::job::description               old_jd_;   // original jd
      saga::job::description               new_jd_;   // new local job description including ssh

      bool                                 use_gsi_;


      void check_ini_    (void);

      saga::job::job create_local_job_ (std::string host = "localhost");


    public:
      // constructor of the job adaptor
      job_cpi_impl  (proxy                           * p, 
                     cpi_info const                  & info,
                     saga::ini::ini const            & glob_ini, 
                     saga::ini::ini const            & adap_ini,
                     TR1::shared_ptr <saga::adaptor>   adaptor);

      // destructor of the job adaptor
      ~job_cpi_impl (void);

      // job functions
      void sync_get_state       (saga::job::state       & ret);
      void sync_get_description (saga::job::description & ret);
      void sync_get_job_id      (std::string            & ret);

      void sync_get_stdin       (saga::job::ostream     & ret);
      void sync_get_stdout      (saga::job::istream     & ret);
      void sync_get_stderr      (saga::job::istream     & ret);

      void sync_checkpoint      (saga::impl::void_t     & ret);
      void sync_migrate         (saga::impl::void_t     & ret,
                                 saga::job::description   jd);
      void sync_signal          (saga::impl::void_t     & ret, 
                                 int                      signal);

      // inherited from saga::task
      void sync_run     (saga::impl::void_t & ret);
      void sync_cancel  (saga::impl::void_t & ret, 
                         double               timeout);
      void sync_suspend (saga::impl::void_t & ret);
      void sync_resume  (saga::impl::void_t & ret);

      void sync_wait    (bool         & ret, 
                         double         timeout);

  };  // class job_cpi_impl

} // namespace ssh_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_SSH_JOB_HPP

