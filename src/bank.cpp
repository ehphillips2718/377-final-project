#include <bank.h>

/**
 * @brief prints account information
 */
void Bank::print_accounts() {
  for (auto& [id, acc] : accounts) {
    {
      // Automatically unlocks when destroyed.
      std::scoped_lock lock {acc.read_lock};
      if (++acc.readers == 1) acc.write_lock.lock();
    }
    
    std::cout << "ID# " << id << " | " << acc.balance << "\n";

    {
      // Automatically unlocks when destroyed.
      std::scoped_lock lock {acc.read_lock};
      if (--acc.readers == 0) acc.write_lock.unlock();
    }
  }

  // Automatically unlocks when destroyed.
  std::scoped_lock lock {bank_lock};
  std::cout << "Success: " << num_succ << " Fails: " << num_fail << "\n";
}

/**
 * @brief helper function to increment the bank variable `num_fail` and log 
 *        message.
 * 
 * @param message message to be logged
 */
void Bank::recordFail(std::string message) {
  // Automatically unlocks when destroyed.
  std::scoped_lock lock {bank_lock};
  std::cout << message << "\n";
  num_fail++;
}

/**
 * @brief helper function to increment the bank variable `num_succ` and log 
 *        message.
 * 
 * @param message message to be logged
 */
void Bank::recordSucc(std::string message) {
  // Automatically unlocks when destroyed.
  std::scoped_lock lock {bank_lock};
  std::cout << message << "\n";
  num_succ++;
}

/**
 * @brief Construct a new Bank::Bank object with N initial accounts.
 * 
 * Initializes the bank with open accounts for account IDs 0 ... N-1.
 * 
 * @param N initial accounts
 */
Bank::Bank(int N) {
  for (int i = 0; i < N; ++i) {
    Account& acc = accounts[i];
    acc.open = true;
  }
}

/**
 * @brief Deposits money into an account.
 * 
 * If the account exists and is open, 
 * [amount] is added to the balance of the account and the following message is logged:
 *  - 'Worker [worker_id] completed ledger [ledger_id]: deposit $[amount] into account [acc_id].'
 * 
 * Otherwise, an error is returned and the following message is logged:
 *  - 'Worker [worker_id] failed to completed ledger [ledger_id]: deposit $[amount] into account [acc_id].'
 * 
 * @param worker_id the ID of the worker (thread)
 * @param ledger_id the ID of the ledger entry
 * @param acc_id the account ID to deposit 
 * @param amount the amount deposited
 * @return int 0 on success, -1 on failure
 */
int Bank::deposit(int worker_id, int ledger_id, int acc_id, int amount) {
  char buffer[100];
  if (accounts.find(acc_id) != accounts.end()) {
    Account& acc = accounts[acc_id];

    // Automatically unlocks when destroyed.
    std::scoped_lock acc_lock {acc.write_lock};
    if (acc.open) {
      acc.balance += amount;
      sprintf(buffer, "Worker %d completed ledger %d: deposit $%d into account %d", worker_id, ledger_id, amount, acc_id);
      recordSucc(buffer);

      return 0;
    }
  }

  sprintf(buffer, "Worker %d failed to complete ledger %d: deposit $%d into account %d", worker_id, ledger_id, amount, acc_id);
  recordFail(buffer);

  return -1;
}

/**
 * @brief Withdraws money from an account.
 * 
 * If the account exists and is open and has at least [amount] as a balance, 
 * [amount] is removed to the balance of the account and the following message is logged:
 *  - 'Worker [worker_id] completed ledger [ledger_id]: deposit $[amount] into account [acc_id].'
 * 
 * Otherwise, an error is returned and the following message is logged:
 *  - 'Worker [worker_id] failed to completed ledger [ledger_id]: deposit $[amount] into account [acc_id].'
 * 
 * @param worker_id the ID of the worker (thread)
 * @param ledger_id the ID of the ledger entry
 * @param acc_id the account ID to withdraw 
 * @param amount the amount withdrawn
 * @return int 0 on success -1 on failure
 */
int Bank::withdraw(int worker_id, int ledger_id, int acc_id, int amount) {
  char buffer[100];
  if (accounts.find(acc_id) != accounts.end()) {
    Account& acc = accounts[acc_id];

    // Automatically unlocks when destroyed.
    std::scoped_lock acc_lock {acc.write_lock};
    if (acc.open && amount <= acc.balance) {
      acc.balance -= amount;
      sprintf(buffer, "Worker %d completed ledger %d: withdraw $%d from account %d", worker_id, ledger_id, amount, acc_id);
      recordSucc(buffer);

      return 0;
    }
  }

  sprintf(buffer, "Worker %d failed to complete ledger %d: withdraw $%d from account %d", worker_id, ledger_id, amount, acc_id);
  recordFail(buffer);

  return -1;
}

/**
 * @brief Transfer money from one account to another.
 * 
 * If both accounts exist and are open and source has at least [amount] as a balance, 
 * [amount] is removed to the balance of the source and added to the balance of destination, 
 * and the following message is logged:
 *  - 'Worker [worker_id] completed ledger [ledger_id]: deposit $[amount] into account [acc_id].'
 * 
 * Otherwise, an error is returned and the following message is logged:
 *  - 'Worker [worker_id] failed to completed ledger [ledger_id]: deposit $[amount] into account [acc_id].'
 * 
 * @param worker_id the ID of the worker (thread)
 * @param ledger_id the ID of the ledger entry
 * @param src_id the account to transfer money out 
 * @param dest_id the account to receive the money
 * @param amount the amount to transfer
 * @return int 0 on success, -1 on error
 */
int Bank::transfer(int worker_id, int ledger_id, int src_id, int dest_id, unsigned int amount) {
  char buffer[100];
  if (src_id != dest_id && accounts.find(src_id) != accounts.end() && accounts.find(dest_id) != accounts.end()) {
    Account& src_acc = accounts[src_id];
    Account& dest_acc = accounts[dest_id];

    // Ensure strict ordering of locks by locking lowest id first; automatically unlocks when destroyed.
    std::scoped_lock acc1_lock {src_id < dest_id ? src_acc.write_lock  : dest_acc.write_lock};
    std::scoped_lock acc2_lock {src_id < dest_id ? dest_acc.write_lock : src_acc.write_lock};
    if (src_acc.open && dest_acc.open && amount <= src_acc.balance) {
      src_acc.balance -= amount;
      dest_acc.balance += amount;
      sprintf(buffer, "Worker %d completed ledger %d: transfer $%d from account %d to account %d", worker_id, ledger_id, amount, src_id, dest_id);
      recordSucc(buffer);

      return 0;
    }
  }

  sprintf(buffer, "Worker %d failed to complete ledger %d: transfer $%d from account %d to account %d", worker_id, ledger_id, amount, src_id, dest_id);
  recordFail(buffer);

  return -1;
}

/**
 * @brief Checks money in an account.
 * 
 * If the account exists and is open, the following message is logged:
 *  - 'Worker [worker_id] completed ledger [ledger_id]: balance of $[acc.balance] in account [acc_id].'
 * 
 * Otherwise, an error is returned and the following message is logged:
 *  - 'Worker [worker_id] failed to completed ledger [ledger_id]: balance of account [acc_id].'
 * 
 * @param worker_id the ID of the worker (thread)
 * @param ledger_id the ID of the ledger entry
 * @param acc_id the account ID to check 
 * @return int 0 on success, -1 on error
 */
int Bank::check_balance(int worker_id, int ledger_id, int acc_id) {
  char buffer[100];
  if (accounts.find(acc_id) != accounts.end()) {
    Account& acc = accounts[acc_id];
    {
      // Automatically unlocks when destroyed.
      std::scoped_lock lock {acc.read_lock};
      if (++acc.readers == 1) acc.write_lock.lock();
    }
    
    if (acc.open) {
      sprintf(buffer, "Worker %d completed ledger %d: balance of $%ld in account %d.", worker_id, ledger_id, acc.balance, acc_id);
      recordSucc(buffer);
    }

    {
      // Automatically unlocks when destroyed.
      std::scoped_lock lock {acc.read_lock};
      if (--acc.readers == 0) acc.write_lock.unlock();
    }

    return -!acc.open;
  }

  sprintf(buffer, "Worker %d failed to complete ledger %d: balance of account %d.", worker_id, ledger_id, acc_id);
  recordFail(buffer);

  return -1;
}

/**
 * @brief Opens an account in the current bank.
 * 
 * If the account is not open or doesn't exist, it is opened or added to the 
 * bank's current accounts and the following message is logged:
 *  - 'Worker [worker_id] completed ledger [ledger_id]: open account [acc_id].'
 * 
 * Otherwise, an error is returned and the following message is logged:
 *  - 'Worker [worker_id] failed to completed ledger [ledger_id]: open account [acc_id].'
 * 
 * @param worker_id the ID of the worker (thread)
 * @param ledger_id the ID of the ledger entry
 * @param acc_id the account ID to open
 * @return int 0 on success, -1 on error 
 */
int Bank::open_account(int worker_id, int ledger_id, int acc_id) {
  char buffer[100];
  if (accounts.find(acc_id) == accounts.end()) {
    Account& acc = accounts[acc_id];

    // Automatically unlocks when destroyed.
    std::scoped_lock acc_lock {acc.write_lock};
    if (!acc.open) {
      acc.open = true;
      sprintf(buffer, "Worker %d completed ledger %d: open account %d.", worker_id, ledger_id, acc_id);
      recordSucc(buffer);
      return 0;
    }
  }

  sprintf(buffer, "Worker %d failed to complete ledger %d: open account %d.", worker_id, ledger_id, acc_id);
  recordFail(buffer);

  return -1;
}

/**
 * @brief Closes an account in the current bank.
 * 
 * If the account exists and is open, it is closed and the following message is logged:
 *  - 'Worker [worker_id] completed ledger [ledger_id]: close account [acc_id].'
 * 
 * Otherwise, an error is returned and the following message is logged:
 *  - 'Worker [worker_id] failed to completed ledger [ledger_id]: close account [acc_id].'
 * 
 * @param worker_id the ID of the worker (thread)
 * @param ledger_id the ID of the ledger entry
 * @param acc_id the account ID to close
 * @return int 0 on success, -1 on error 
 */
int Bank::close_account(int worker_id, int ledger_id, int acc_id) {
  char buffer[100];
  if (accounts.find(acc_id) != accounts.end()) {
    Account& acc = accounts[acc_id];

    // Automatically unlocks when destroyed.
    std::scoped_lock acc_lock {acc.write_lock};
    if (acc.open) {
      acc.open = false;
      sprintf(buffer, "Worker %d completed ledger %d: close account %d.", worker_id, ledger_id, acc_id);
      recordSucc(buffer);
      return 0;
    }
  }

  sprintf(buffer, "Worker %d failed to complete ledger %d: close account %d.", worker_id, ledger_id, acc_id);
  recordFail(buffer);

  return -1;
}