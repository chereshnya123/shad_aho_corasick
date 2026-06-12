def get_all_substrings(input):
  substrings = set()
  for i in range(len(input)):
    for j in range(i, len(input)):
      substrings.add(input[i:j+1])
  
  return substrings

N = int(input())
strings = []
for _ in range(N):
  strings.append(input())

substr_set = get_all_substrings(strings[0])

for s in strings[1:]:
  substr_set = substr_set & get_all_substrings(s)

print(max(substr_set, key = lambda x: len(x)))