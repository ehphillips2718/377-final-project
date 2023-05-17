#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>


#include "ledger.h"

using namespace std;


// test correct init 
TEST(BankTest, Test1) {
    Bank *bank = new Bank(10);

    stringstream output;
    streambuf* oldCoutStreamBuf = cout.rdbuf(); // save cout's streambuf
    cout.rdbuf(output.rdbuf()); // redirect cout to stringstream
    bank->print_accounts(); // call the print method
    cout.rdbuf(oldCoutStreamBuf); // restore cout's original streambuf

    string line = "";
    string stats("Success: 0 Fails: 0");

    int init_account_balance_to_zero = 0;
    int init_stats_to_0 = 0;
    
    while (getline(output, line)) {
      if (line.compare(8, 9, "0") == 0) {
        init_account_balance_to_zero++;
      }

      if (line.compare(0, 19, stats) == 0) {
        init_stats_to_0++;
      }
    }

    EXPECT_EQ(init_account_balance_to_zero, 10) << "Make sure to initialize the account balance to 0";
    EXPECT_EQ(init_stats_to_0, 1);

    delete bank;
}


TEST(BankTest, Test2) {
    Bank *bank = new Bank(10);

    // capture out
    stringstream output;
    streambuf* oldCoutStreamBuf = cout.rdbuf(); // save cout's streambuf
    cout.rdbuf(output.rdbuf()); // redirect cout to stringstream

    // add ballance
    bank->deposit(0, 0, 1, 100);
    bank->deposit(0, 0, 3, 100);
    bank->deposit(0, 0, 5, 100);
    
    cout.rdbuf(oldCoutStreamBuf); // restore cout's original streambuf
    
    EXPECT_EQ(bank->accounts[1].balance, 100);
    EXPECT_EQ(bank->accounts[3].balance, 100);
    EXPECT_EQ(bank->accounts[5].balance, 100);
    EXPECT_EQ(bank->accounts[8].balance, 0);

    delete bank;
}

TEST(BankTest, Test3) {
    Bank *bank = new Bank(10);

    // capture out
    stringstream output;
    streambuf* oldCoutStreamBuf = cout.rdbuf(); // save cout's streambuf
    cout.rdbuf(output.rdbuf()); // redirect cout to stringstream
    

    // add ballance
    bank->deposit(0, 0, 1, 100);
    bank->deposit(0, 0, 3, 100);
    bank->deposit(0, 0, 5, 100);

    int withdraw1 = bank->withdraw(0, 0, 1, 50);
    int withdraw2 = bank->withdraw(0, 0, 0, 50);

    cout.rdbuf(oldCoutStreamBuf); // restore cout's original streambuf
    
    EXPECT_EQ(withdraw1, 0);
    EXPECT_EQ(withdraw2, -1);

    delete bank;
}

TEST(BankTest, Test4) {
    Bank *bank = new Bank(10);

    // capture out
    stringstream output;
    streambuf* oldCoutStreamBuf = cout.rdbuf(); // save cout's streambuf
    cout.rdbuf(output.rdbuf()); // redirect cout to stringstream
    

    // add ballance
    bank->deposit(0, 0, 1, 100);
    bank->deposit(0, 0, 3, 100);
    bank->deposit(0, 0, 5, 100);

    int transfer1 = bank->transfer(0, 0, 1, 0, 50);
    int transfer2 = bank->transfer(0, 0, 6, 7, 50);

    cout.rdbuf(oldCoutStreamBuf); // restore cout's original streambuf
    
    EXPECT_EQ(transfer1, 0);
    EXPECT_EQ(transfer2, -1);
    EXPECT_TRUE(bank->accounts[1].balance == 50 && bank->accounts[0].balance == 50);

    delete bank;
}

TEST(BankTest, Test5) {
    Bank *bank = new Bank(10);

    // capture out
    stringstream output;
    streambuf* oldCoutStreamBuf = cout.rdbuf(); // save cout's streambuf
    cout.rdbuf(output.rdbuf()); // redirect cout to stringstream
    

    // add ballance
    bank->deposit(0, 0, 1, 100);

    int transfer1 = bank->transfer(0, 0, 1, 1, 50);

    cout.rdbuf(oldCoutStreamBuf); // restore cout's original streambuf
    
    EXPECT_EQ(transfer1, -1);

    delete bank;
}

TEST(BankTest, Test6) {
    Bank *bank = new Bank(10);

    // capture out
    
    int ret;
    if ((ret = bank->accounts[0].write_lock.try_lock()) == 0) {
      // Mutex was successfully locked
      bank->accounts[0].write_lock.unlock();
    } else {
      // err
    }

    ASSERT_NE(ret, 22) << "Forgot to initialize account lock?";

    if ((ret = bank->bank_lock.try_lock()) == 0) {
      // Mutex was successfully locked
      bank->bank_lock.unlock();
    } else {
      // err
    }

    ASSERT_NE(ret, 22) << "Forgot to initialize bank global lock?";

    delete bank;
}

TEST(BankTest, Test7) {
    Bank *bank = new Bank();

    // capture out
    stringstream output;
    streambuf* oldCoutStreamBuf = cout.rdbuf(); // save cout's streambuf
    cout.rdbuf(output.rdbuf()); // redirect cout to stringstream

    // open account and add balance
    int deposit1 = bank->deposit(0, 0, 1, 100);
    int open1 = bank->open_account(0, 0, 1);
    int deposit2 = bank->deposit(0, 0, 1, 100);
    int open2 = bank->open_account(0, 0, 1);

    cout.rdbuf(oldCoutStreamBuf); // restore cout's original streambuf
    
    EXPECT_EQ(deposit1, -1);
    EXPECT_EQ(open1, 0);
    EXPECT_EQ(deposit2, 0);
    EXPECT_EQ(open2, -1);
    EXPECT_TRUE(bank->accounts[1].balance == 100);

    delete bank;
}

TEST(BankTest, Test8) {
    Bank *bank = new Bank(10);

    // capture out
    stringstream output;
    streambuf* oldCoutStreamBuf = cout.rdbuf(); // save cout's streambuf
    cout.rdbuf(output.rdbuf()); // redirect cout to stringstream

    // add balance and close account
    int deposit1 = bank->deposit(0, 0, 0, 100);
    int close1 = bank->close_account(0, 0, 0);
    int deposit2 = bank->deposit(0, 0, 0, 100);
    int close2 = bank->close_account(0, 0, 0);

    cout.rdbuf(oldCoutStreamBuf); // restore cout's original streambuf
    
    EXPECT_EQ(deposit1, 0);
    EXPECT_EQ(close1, 0);
    EXPECT_EQ(deposit2, -1);
    EXPECT_EQ(close2, -1);

    delete bank;
}

TEST(BankTest, Test9) {
    Bank *bank = new Bank(10);

    // capture out
    stringstream output;
    streambuf* oldCoutStreamBuf = cout.rdbuf(); // save cout's streambuf
    cout.rdbuf(output.rdbuf()); // redirect cout to stringstream

    // add balance and close account
    int check1 = bank->check_balance(0, 0, 0);
    int deposit1 = bank->deposit(0, 0, 0, 100);
    int check2 = bank->check_balance(0, 0, 0);
    int close = bank->close_account(0, 0, 0);
    int check3 = bank->check_balance(0, 0, 0);

    cout.rdbuf(oldCoutStreamBuf); // restore cout's original streambuf
    
    EXPECT_EQ(check1, 0);
    EXPECT_EQ(deposit1, 0);
    EXPECT_EQ(check2, 0);
    EXPECT_EQ(close, 0);
    EXPECT_EQ(check3, -1);

    delete bank;
}


/// test load 
TEST(LedgerTest, Test1){
    bool done = false;
	  std::ifstream file {"short_ledger.txt"};
	  std::mutex file_lock;

  	int ledger_id = 0;
	  std::queue<Ledger> ledger;
	  std::mutex ledger_lock;
	  std::condition_variable empty, fill;

    load_ledger(std::ref(done), std::ref(ledger_id), std::ref(file), std::ref(file_lock), 
                std::ref(ledger), std::ref(ledger_lock), std::ref(empty), std::ref(fill));
    EXPECT_EQ(ledger.size(), 4);
}


TEST(LedgerTest, Test2) {

    // capture out
    stringstream output;
    streambuf* oldCoutStreamBuf = cout.rdbuf(); // save cout's streambuf
    cout.rdbuf(output.rdbuf()); // redirect cout to stringstream
  
    InitBank(1, "ledger.txt");

    cout.rdbuf(oldCoutStreamBuf); // restore cout's original streambuf

    string line = "";
    string stats = "Success: 4 Fails: 6";

    int w0{0};
    int succ{0}, fails{0};

    while (getline(output, line)) {
      if (line.compare(0, 8, "Worker 0") == 0) {
        w0++;
      }

      if (line.compare(0, 19, stats) == 0) {
        size_t colon_pos = line.find_first_of(":");
        string success_str = line.substr(colon_pos + 2);

        size_t space_pos = success_str.find_first_of(" ");
        string success_count_str = success_str.substr(0, space_pos);
        succ += stoi(success_count_str);
        string fails_str = line.substr(line.find("Fails: ") + 7);
        fails += stoi(fails_str);
      }
    }

    ASSERT_EQ(w0, 10);
    ASSERT_EQ(succ, 4);
    ASSERT_EQ(fails, 6);
    
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
