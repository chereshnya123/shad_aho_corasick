from C.draft import calculate_z_function

def is_tripple_exist(s, len):
  for start in range(len(s) - 3*len + 1):
    z_f = calculate_z_function(s[start:])
    count = 1
    last_end = start + len
    
    for j in range(last_end, len(z_f)):
      if z_f[j] >= len and last_end <= j + start:
        count += 1
        last_end = j + start + len
        if count >= 3:
          return s[start: start+len]
  
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