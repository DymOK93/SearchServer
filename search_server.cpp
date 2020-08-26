#include "search_server.h"
#include "iterator_range.h"
#include "parse.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>

std::vector<string_view> SplitIntoWords(std::string_view line, char delim = ' ') {
	std::vector<string_view> words;
	line = Strip(line);														//Удаляем начальные и конечные пробелы
	for (;;) {
		size_t space_begin(line.find(delim));
		words.push_back(line.substr(0, space_begin));
		size_t space_end = line.find_first_not_of(delim, space_begin);

		if (space_end == line.npos) {
			break;
		}
		else {
			line.remove_prefix(space_end);
		}
	}
	return words;
}


SearchServer::SearchServer(std::istream& document_input) {
  UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(std::istream& document_input) {

	Synchronized<InvertedIndex> new_index;

	for (string current_document; getline(document_input, current_document); ) {
		new_index.GetAccess().ref.Add(move(current_document));
	}

	index = std::move(new_index);
}

void SearchServer::AddQueriesStream(
	std::istream& query_input, std::ostream& search_results_output
) const noexcept {
	query_handlers.push(
		std::async(
			&SearchServer::ProccessQuery,
			this, 
			std::ref(query_input),
			std::ref(search_results_output)
		));
}

void SearchServer::ProccessQuery(
	istream& query_input,
	ostream& search_results_output 
) const noexcept {

	for (std::string current_query; getline(query_input, current_query); ) {

		const size_t index_docid{ index.GetAccess().ref.GetDocid() };
		std::vector<pair<size_t, size_t>> search_results(index_docid);

		for (const auto& word : SplitIntoWords(current_query)) {

			const auto [raw_relevance, relevance_validity] {  index.GetAccess().ref.Lookup(word) };

			if (relevance_validity) {
				for (const auto [docid, hitcount] : raw_relevance->second) {
					search_results[docid].first = docid;
					search_results[docid].second += hitcount;
				}
			}
		}

		const size_t range_length = basic_range_length >= index_docid ? index_docid : basic_range_length;

		partial_sort(
			begin(search_results),
			begin(search_results) + range_length,
			end(search_results),
			[](const std::pair<size_t, size_t>& lhs, const std::pair<size_t, size_t>& rhs) {
				int64_t lhs_docid = lhs.first;
				auto lhs_hit_count = lhs.second;
				int64_t rhs_docid = rhs.first;
				auto rhs_hit_count = rhs.second;
				return make_pair(lhs_hit_count, -lhs_docid) > make_pair(rhs_hit_count, -rhs_docid);
			}
		);
		OutputResult(current_query, search_results, search_results_output);
	}
}

void SearchServer::OutputResult(
	std::string_view query,
	const InvertedIndex::RelevanceBase& search_results,
	std::ostream& search_results_output
	) const noexcept {

	search_results_output << query << ':';
	for (const auto [docid, hitcount] : Head(search_results, basic_range_length)) {
		if (hitcount) {
			search_results_output << " {"
				<< "docid: " << docid << ", "
				<< "hitcount: " << hitcount << '}';
		}
	}
	search_results_output << endl;
}

void InvertedIndex::Add(std::string document) {
	const size_t docid = docs.size();

	docs.push_back(std::move(document));

	std::unordered_map<std::string_view, size_t> words_count;

	for (auto& word : SplitIntoWords(docs.back())) {
		++words_count[word];
  }
	for (auto& [word, count] : words_count) {
		index[word].push_back({ docid, count });
	}
}

InvertedIndex::RelevanceInfo InvertedIndex::Lookup(std::string_view word) const noexcept {
	auto it{ index.find(word) };
	return { it, it != index.end() };
}
