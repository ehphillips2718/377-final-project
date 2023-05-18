# CS 377 Final Project: Project 4 Extension

## Functionality

### Running the App

To run the program, you need to execute

```
./bank_app <num_workers> <ledger_file>
```

from the command line. `<num_workers>` tells the app both how many threads will read from `<ledger_file>` and how many worker threads will operate on the bank concurrently. `<ledger_file>` is a .txt file with instructions on what operations to run on each account.

Alternatively, 

```
./bank_test 
```

can be run to perform a series of tests on the bank app. These tests are defined as follows:

```
    BankTest -- Test1: Makes sure all accounts' balances are initialized to 0.
    BankTest -- Test2: Makes sure deposit works properly.
    BankTest -- Test3: Makes sure withdraw works properly.
    BankTest -- Test4: Makes sure transfer works properly.
    BankTest -- Test5: Makes sure it is not valid to transfer from one account to the same account.
    BankTest -- Test6: Makes sure account locks and global locks work properly.
    BankTest -- Test7: Makes sure open account works properly.
    BankTest -- Test8: Makes sure close account works properly.
    BankTest -- Test9: Makes sure check balance works properly.

    LedgerTest -- Test1: Makes sure a short test ledger can be properly loaded into the buffer.
    LedgerTest -- Test2: Makes sure that we can load a ledger from a file produce the correct outputs.
```

### Text File Structure

The structure of `ledger.txt` is as follows. Each line is defined by 4 integers where the value indicates the following:

```
FROM_ID TO_ID AMOUNT MODE
```

`FROM_ID` is the account ID we would like to act on. `TO_ID` is only used on transfers and is the account ID to transfer money to. `AMOUNT` is the amount to deposit, withdraw, or transfer. Finally, `MODE` is the action where:

```
0 => Deposit
1 => Withdraw
2 => Transfer
3 => Check Balance
4 => Open Account
5 => Close Account
```

## Bank and Account Functions

### Ledger

* `InitBank()` is the entry to the bank. It initiallizes a `Bank` object with `10` accounts, then creates `num_workers` threads to parse the file given by `filename` and `num_workers` threads to perform the work specified by the items in the bounded ledger.
* `load_ledger()` takes in a boolean `done` representing if we have finished reading the file stream, the current ledger id `ledger_id`, a file stream `file` and lock for it `stream_lock`, a buffer ledger `ledger` with a lock and two condition variables for it `ledger_lock`, `empty`, and `fill` respectively. It parses the file and puts ledger instances from the file into the bounded buffer `ledger`.
* `worker()` takes in the bank to act upon `Bank`, a boolean `done` representing if we have finished reading the file stream, an integer representing what worker this thread is `worker_id`, a buffer ledger `ledger` with a lock and two condition variables for it `ledger_lock`, `empty`, and `fill` respectively. It parses the file and ledger instances from the file into the bounded buffer `ledger`. It takes ledger instances from the bounded buffer `ledger` and attempt to perform the specified ledger item on the given `bank`.

### Bank

* Bank constructor. There is an empty default constructor that simply constructs a bank with no accounts. The other constructor takes in an integer `N` and initializes the first `N` accounts of the Bank.
* Bank destructor. Empty due to RAII freeing all memory and destroying all locks for us.
* `deposit()`: Deposits money into an account. If the account exists and is open, [`amount`] is added to the balance of the account and the following message is logged: - `Worker [worker_id] completed ledger [ledger_id]: deposit $[amount] into account [acc_id].` Otherwise, an error is returned and the following message is logged: - `Worker [worker_id] failed to completed ledger [ledger_id]: deposit $[amount] into account [acc_id].`
* `withdraw()`: Withdraws money from an account. If the account exists and is open and has at least [`amount`] as a balance, [`amount`] is removed to the balance of the account and the following message is logged:  - `Worker [worker_id] completed ledger [ledger_id]: deposit $[amount] into account [acc_id].`  Otherwise, an error is returned and the following message is logged: - `Worker [worker_id] failed to completed ledger [ledger_id]: deposit $[amount] into account [acc_id].`
* `transfer()`: Transfer money from one account to another. If both accounts exist and are open and source has at least [`amount`] as a balance, [`amount`] is removed to the balance of the source and added to the balance of destination, and the following message is logged: - `Worker [worker_id] completed ledger [ledger_id]: deposit $[amount] into account [acc_id].` Otherwise, an error is returned and the following message is logged: - `Worker [worker_id] failed to completed ledger [ledger_id]: deposit $[amount] into account [acc_id].`
* `check_balance()`: Checks money in an account. If the account exists and is open, the following message is logged: - `Worker [worker_id] completed ledger [ledger_id]: balance of $[acc.balance] in account [acc_id].` Otherwise, an error is returned and the following message is logged: - `Worker [worker_id] failed to completed ledger [ledger_id]: balance of account [acc_id].`
* `open_account()`: Opens an account in the current bank. If the account is not open or doesn't exist, it is opened or added to the bank's current accounts and the following message is logged: - `Worker [worker_id] completed ledger [ledger_id]: open account [acc_id].` Otherwise, an error is returned and the following message is logged: - `Worker [worker_id] failed to completed ledger [ledger_id]: open account [acc_id].`
* `close_account()`: Closes an account in the current bank. If the account exists and is open, it is closed and the following message is logged: - `Worker [worker_id] completed ledger [ledger_id]: close account [acc_id].` Otherwise, an error is returned and the following message is logged: - `Worker [worker_id] failed to completed ledger [ledger_id]: close account [acc_id].`

## Example Results

Consider the following line in `ledger.txt`:

```
...
4 3 400 2
...
```

this line says take $400 out of `Account[4]` and add it to `Account[3]`.

Running the bank app with `./bank_app 10 pressure_test.txt` should yield the following output (with varying order and worker ids for completing the ledgers):

```
ID# 0 | 0
ID# 1 | 0
ID# 2 | 0
ID# 3 | 0
ID# 4 | 0
ID# 5 | 0
ID# 6 | 0
ID# 7 | 0
ID# 8 | 0
ID# 9 | 0
Success: 0 Fails: 0
Worker 0 completed ledger 0: deposit $38 into account 8
Worker 1 completed ledger 2: deposit $9 into account 1
Worker 1 failed to complete ledger 3: withdraw $95 from account 3
Worker 0 failed to complete ledger 1: transfer $237 from account 8 to account 9
Worker 0 completed ledger 5: balance of $0 in account 0.
Worker 1 failed to complete ledger 6: transfer $233 from account 0 to account 7
Worker 2 failed to complete ledger 4: withdraw $29 from account 2
Worker 0 completed ledger 7: deposit $334 into account 3
Worker 1 failed to complete ledger 8: withdraw $54 from account 2
Worker 2 completed ledger 9: balance of $9 in account 1.
Worker 1 completed ledger 11: deposit $38 into account 8
Worker 2 completed ledger 12: close account 9.
Worker 3 failed to complete ledger 10: transfer $280 from account 5 to account 8
Worker 0 failed to complete ledger 15: open account 6.
Worker 2 completed ledger 14: deposit $110 into account 4
Worker 1 failed to complete ledger 13: transfer $237 from account 8 to account 9
Worker 3 completed ledger 17: open account 14.
Worker 1 completed ledger 18: open account 35.
Worker 2 failed to complete ledger 20: withdraw $29 from account 2
Worker 3 completed ledger 19: balance of $0 in account 6.
Worker 3 completed ledger 21: withdraw $95 from account 3
Worker 4 completed ledger 16: deposit $9 into account 1
Worker 1 completed ledger 24: balance of $110 in account 4.
Worker 3 failed to complete ledger 23: transfer $233 from account 0 to account 7
Worker 2 completed ledger 26: open account 24.
Worker 0 completed ledger 25: deposit $38 into account 8
Worker 2 completed ledger 29: deposit $9 into account 1
Worker 4 completed ledger 27: close account 6.
Worker 0 failed to complete ledger 30: transfer $237 from account 8 to account 9
Worker 3 completed ledger 28: deposit $110 into account 4
Worker 0 completed ledger 33: deposit $334 into account 3
Worker 4 failed to complete ledger 34: open account 0.
Worker 2 failed to complete ledger 35: withdraw $29 from account 2
Worker 5 failed to complete ledger 22: withdraw $54 from account 2
Worker 0 completed ledger 38: deposit $334 into account 3
Worker 6 completed ledger 32: balance of $0 in account 7.
Worker 0 completed ledger 40: open account 20.
Worker 3 failed to complete ledger 36: transfer $280 from account 5 to account 8
Worker 5 failed to complete ledger 41: transfer $237 from account 8 to account 9
Worker 0 failed to complete ledger 42: transfer $233 from account 0 to account 7
Worker 2 failed to complete ledger 37: withdraw $54 from account 2
Worker 1 failed to complete ledger 31: open account 5.
Worker 3 completed ledger 43: open account 10.
Worker 4 failed to complete ledger 39: close account 11.
Worker 7 completed ledger 44: withdraw $95 from account 3
Worker 7 completed ledger 45: deposit $38 into account 8
Worker 5 failed to complete ledger 46: balance of account 12.
Worker 6 failed to complete ledger 47: transfer $233 from account 0 to account 7
Worker 5 completed ledger 48: deposit $334 into account 3
Worker 2 completed ledger 51: balance of $152 in account 8.
Worker 1 completed ledger 50: deposit $9 into account 1
Worker 3 failed to complete ledger 55: transfer $280 from account 5 to account 8
Worker 1 failed to complete ledger 57: withdraw $29 from account 2
Worker 8 completed ledger 53: deposit $110 into account 4
Worker 3 completed ledger 61: withdraw $95 from account 3
Worker 4 failed to complete ledger 62: transfer $280 from account 5 to account 8
Worker 5 completed ledger 52: open account 123.
Worker 1 completed ledger 59: close account 10.
Worker 6 failed to complete ledger 58: balance of account 200.
Worker 2 completed ledger 54: close account 24.
Worker 8 failed to complete ledger 63: withdraw $54 from account 2
Worker 9 completed ledger 60: deposit $110 into account 4
Worker 7 completed ledger 56: open account 13.
Worker 3 failed to complete ledger 64: balance of account 15.
ID# 0 | 0
ID# 1 | 36
ID# 2 | 0
ID# 3 | 1051
ID# 4 | 440
ID# 5 | 0
ID# 7 | 0
ID# 8 | 152
ID# 13 | 0
ID# 14 | 0
ID# 20 | 0
ID# 35 | 0
ID# 123 | 0
Success: 36 Fails: 28
```