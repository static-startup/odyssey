# ifndef UTILS_H
# define UTILS_H

static std::vector<std::string> split_into_args(std::string str) {
	std::vector<std::string> result(1);
	bool in_quotation = false;
	for(int i = 0; i < str.length(); i++) {
		if(i != 0) {
			if(str[i] == '"') {
				if(str[i - 1] == '\\') {
					result[result.size() - 1].pop_back();
				} else {
					in_quotation = !in_quotation;
					continue;
				}
			}
			if(i != str.length() - 1
				&& str[i + 1] != ' '
				&& str[i] == ' '
				&& !in_quotation){

				if(str[i - 1] == '\\') {
					result[result.size() - 1].pop_back();
				}  else {
					result.push_back("");
					continue;
				}
			}
		}
		result[result.size() - 1] += str[i];
	}
	return result;
}

# endif
