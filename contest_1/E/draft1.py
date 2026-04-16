from C.draft import calculate_z_function

def is_tripple_beginning(s, len):
  patterns_count = dict()
  for start in range(len(s) - 3*len + 1):
    z = calculate_z_function(s[start:])
    
    for j in range(1, len(z)):
      if z[j] <= j:
        if z[j] not in patterns_count:
          patterns_count[z[j]] = [2, j + z[j]]
        elif patterns_count[z[j]][1]:
          patterns_count[z[j]][0] += 1
          patterns_count[z[j]][1] = j + z[j]
        
  
  return ""
    

def calc_max_tripple_substring(s):
  min_len = 1
  max_len = len(s) // 3
  max_tripple = ""
  while min_len <= max_len:
    mid = (min_len + max_len) // 2
    
    max_tripple = is_tripple_exist(mid, s)
    if max_tripple != "":
      min_len = mid +1
    else:
      max_len = mid - 1

  return max_tripple

if __name__ == '__main__':
  while True:
    s = input()
    print(calc_max_tripple_substring(s))