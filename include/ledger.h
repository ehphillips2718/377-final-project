#ifndef _LEDGER_H
#define _LEDGER_H

#include <bank.h>

#define MAX_SIZE 5

struct Ledger {
	int from;
	int to;
	int amount;
  	int mode;
	int ledgerID;
};

void InitBank(int num_workers, std::string filename);
void load_ledger(bool& done, int& ledger_id, std::ifstream& file, std::mutex& stream_lock, 
				 std::queue<Ledger>& ledger, std::mutex& ledger_lock, std::condition_variable& empty, std::condition_variable& fill);
void worker(Bank& bank, bool& done, int worker_id, 
			std::queue<Ledger>& ledger, std::mutex& ledger_lock, std::condition_variable& empty, std::condition_variable& fill);

#endif