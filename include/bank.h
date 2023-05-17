#ifndef _BANK_H
#define _BANK_H

#include <stdlib.h>
#include <fstream>
#include <string>
#include <sys/wait.h>   /* for wait() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <sys/mman.h>   /* for mmap() ) */
#include <semaphore.h>  /* for sem */
#include <assert.h>		/* for assert */
#include <iostream>     /* for cout */
#include <list>
#include <array>

#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>
#include <queue>

struct Account {
  bool open {false};
  long balance {0};
  int  readers {0};

  std::mutex read_lock;
  std::mutex write_lock;
};


class Bank {
  private:
    int num_succ {0};
    int num_fail {0};
    
  public:
    // empty constructor/destructor due to RAII (initialization and destruction is handled for us)
    Bank() {}; 
    ~Bank() {};

    // initialize accounts 0 ... N-1 
    Bank(int N);
    
    int deposit (int worker_id, int ledger_id, int acc_id, int amount);
    int withdraw(int worker_id, int ledger_id, int acc_id, int amount);
    int transfer(int worker_id, int ledger_id, int src_id, int dest_id, unsigned int amount);
    int check_balance(int worker_id, int ledger_id, int acc_id);
    int open_account (int worker_id, int ledger_id, int acc_id);
    int close_account(int worker_id, int ledger_id, int acc_id);
    
    void print_accounts();
    void recordSucc(std::string message);
    void recordFail(std::string message);

    std::mutex bank_lock;
    std::map<int, Account> accounts;
};

#endif