#pragma once

#include "syncronized.h"
#include <future>
#include <thread>
#include <mutex>
#include <istream>
#include <ostream>
#include <vector>
#include <queue>
#include <functional>
#include <string>
#include <unordered_map>

class InvertedIndex {
public:
	using RelevanceBase = std::vector<std::pair<size_t, size_t>>;
	using RelevanceDictionary = std::unordered_map<std::string_view, RelevanceBase>;
	using RelevanceInfo = std::pair<typename RelevanceDictionary::const_iterator, bool>;
public:
  void Add(std::string document);

  RelevanceInfo Lookup(std::string_view word) const noexcept;

  const std::string& GetDocument(size_t docid) const {
	  return docs[docid];
  }

  size_t GetDocid() const noexcept {
	  return docs.size();
  }
private:
	RelevanceDictionary index;
	std::deque<std::string> docs;
};

class SearchServer {
public:
	SearchServer() = default;
	~SearchServer() = default;

	explicit SearchServer(std::istream& document_input);
	void UpdateDocumentBase(std::istream& document_input);
	void AddQueriesStream(std::istream& query_input, std::ostream& search_results_output) const noexcept;
private:
	void ProccessQuery(std::istream& query_input, std::ostream& search_results_output) const noexcept;
	void OutputResult(std::string_view query, const InvertedIndex::RelevanceBase& result, std::ostream& search_results_output) const noexcept;
private:
	Synchronized<InvertedIndex> index;
	mutable std::queue<std::future<void>> query_handlers;
private:
	static const size_t basic_range_length{ 5 };
	static const size_t docid_max{ 50000 };
};
