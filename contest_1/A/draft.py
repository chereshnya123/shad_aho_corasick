def calculate_prefix_func(s):
    prefix_f = [0] * len(s)  # prefix_f[i] = longest border of s[:i+1]
    border_len = 0

    for i in range(1, len(s)):
        if s[border_len] == s[i]:
            border_len += 1
            prefix_f[i] = border_len
            continue

        while border_len != 0:
            border_len = prefix_f[border_len - 1]
            if s[border_len] == s[i]:
                border_len += 1
                prefix_f[i] = border_len
                break

    return prefix_f

if __name__ == "__main__":
    s = input()
    print(*calculate_prefix_func(s))
    
