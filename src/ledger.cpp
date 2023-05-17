#include <ledger.h>

/**
 * @brief Creates a new bank object and sets up workers to read from the file and execute the ledger.
 *  
 * @param num_workers number of workers to create for both reading and executing the ledger
 * @param filename file to read
 */
void InitBank(int num_workers, std::string filename) {
	Bank bank = Bank(10);

	// File reading variables
	bool done = false;
	std::ifstream file {filename};
	std::mutex file_lock;

	// Ledger variables
	int ledger_id = 0;
	std::queue<Ledger> ledger;
	std::mutex ledger_lock;
	std::condition_variable empty, fill;

	// Thread arrays
	std::thread rthreads[num_workers];
	std::thread wthreads[num_workers];

	bank.print_accounts();
	// Initializes all reader and writer threads and then joins them
	for (int i = 0; i < num_workers; ++i) {
		rthreads[i] = std::thread(load_ledger, std::ref(done), std::ref(ledger_id), std::ref(file), std::ref(file_lock),
								  std::ref(ledger), std::ref(ledger_lock), std::ref(empty), std::ref(fill));
		wthreads[i] = std::thread(worker, std::ref(bank), std::ref(done), i, 
								  std::ref(ledger), std::ref(ledger_lock), std::ref(empty), std::ref(fill));
	}
	for (auto& thread : rthreads) thread.join();
	for (auto& thread : wthreads) thread.join();
	bank.print_accounts();
}

/**
 * @brief Parse a ledger file and store each line into a list
 * 
 * @param done boolean representing if we have finished reading the file stream
 * @param ledger_id current ledger id
 * @param file file stream to parse from
 * @param stream_lock mutex lock around the stream
 * @param ledger buffer ledger
 * @param ledger_lock mutex lock around the ledger buffer
 * @param empty condition variable for emptying the ledger buffer 
 * @param fill condition variable for filling the ledger buffer 
 */
void load_ledger(bool& done, int& ledger_id, std::ifstream& file, std::mutex& stream_lock, 
				 std::queue<Ledger>& ledger, std::mutex& ledger_lock, std::condition_variable& empty, std::condition_variable& fill) {
	// Automatically unlocks when destroyed.
	std::unique_lock<std::mutex> file_lock {stream_lock};
	int f, t, a, m;
	while (file >> f >> t >> a >> m) {
		file_lock.unlock();
		{
			// Automatically unlocks when destroyed.
			std::unique_lock<std::mutex> lock {ledger_lock};
			while (ledger.size() == MAX_SIZE) empty.wait(lock);
			ledger.push({f, t, a, m, ledger_id++});
			fill.notify_one();
		}
		file_lock.lock();
	}

	done = true;
}

/**
 * @brief Remove items from the list and execute the instruction.
 * 
 * @param bank bank to process the information from
 * @param done boolean representing if we have finished reading the file stream
 * @param worker_id id of the worker processing 
 * @param ledger buffer ledger
 * @param ledger_lock mutex lock around the ledger buffer
 * @param empty condition variable for emptying the ledger buffer  
 * @param fill condition variable for filling the ledger buffer 
 */
void worker(Bank& bank, bool& done, int worker_id, 
			std::queue<Ledger>& ledger, std::mutex& ledger_lock, std::condition_variable& empty, std::condition_variable& fill) {
	// Automatically unlocks when destroyed.
	std::unique_lock<std::mutex> lock {ledger_lock};
	while (!done || ledger.size()) {
		while (ledger.empty()) fill.wait(lock);
		Ledger l = ledger.front();
		ledger.pop();
		empty.notify_one();

		lock.unlock();
		switch (l.mode) {
			case 0: bank.deposit      (worker_id, l.ledgerID, l.from,       l.amount); break;
			case 1:	bank.withdraw     (worker_id, l.ledgerID, l.from,       l.amount); break;
			case 2: bank.transfer     (worker_id, l.ledgerID, l.from, l.to, l.amount); break;
			case 3: bank.check_balance(worker_id, l.ledgerID, l.from                ); break;
			case 4: bank.open_account (worker_id, l.ledgerID, l.from                ); break;
			case 5: bank.close_account(worker_id, l.ledgerID, l.from                ); break;
		}
		lock.lock();
	}
}
