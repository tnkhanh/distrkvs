#include <string>
#include <iostream>

#include "hash_id.h"

namespace distrkvs {
class HashIdTest {
  int nexts(std::string& s) {
    for (int i=s.size() - 1; i>=0; --i)
      if (s[i] < 'z') {
        s[i]++;
        for (int j=i+1; j<(int)s.size(); ++j) s[j] = 'a';
        return 1;
      }

    return 0;
  }
 public:
  int test1() {
    int ct = 0;
    std::string s("aaaa");
    while (true) {
      HashId h1 = HashId(KeyWrapper(&s));
      for (int i=0; i < 256; ++i) {
        HashId actual = h1.add_power_of_2(i),
               expected = h1 + HashId::power_of_2(i);
        if (!(actual == expected)) {
          std::cout << s << " " << i << "add\n";
          std::cout << h1.str() << "\n" << actual.str() << "\n" << expected.str() << "\n";
          std::cout << HashId::power_of_2(i).str() << "\n";
          return 0;
        }

        actual = h1.subtract_power_of_2(i);
        expected = h1 - HashId::power_of_2(i);
        if (!(actual==expected)) {
          std::cout << s << " " << i << "subtract\n";
          std::cout << h1.str() << "\n" << actual.str() << "\n" << expected.str() << "\n";
          std::cout << HashId::power_of_2(i).str() << "\n";
          std::cout << HashId::power_of_2(i).negate().str() << "\n";
          return 0;
        }
      }

      ++ct;
      if (!nexts(s)) break;
    }
    std::cout << "Count: " <<ct<<"\n";
    return 1;
  }
};
}  // namespace distrkvs

int main() {
  distrkvs::HashIdTest t1;
  std::cout << "Test 1: " << t1.test1() << "..\n";
  
  return 0;
}
