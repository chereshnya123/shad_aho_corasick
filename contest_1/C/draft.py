def get_longest_common_prefix(s, t):
    common_prefix_len = 0
    for i in range(min(len(s), len(t))):
        if s[i] != t[i]:
            break
        common_prefix_len += 1

    return common_prefix_len


def calculate_z_function(s):
    z_func = [0] * len(s)
    rightest_block = 0
    for i in range(1, len(s)):
        # 1. End of rightest block is lefter than i
        if rightest_block + z_func[rightest_block] <= i:
            z_func[i] = get_longest_common_prefix(s[i:], s)
        # 2. Rightest block don't contain whole preprocessed z-block
        elif (z_func[i - rightest_block] + (i - rightest_block)) >= z_func[
            rightest_block
        ]:
            known_len = z_func[rightest_block] + rightest_block - i
            z_func[i] = known_len + get_longest_common_prefix(
                s[known_len:],
                s[rightest_block + z_func[rightest_block] :],
            )
        else:
            z_func[i] = z_func[i - rightest_block]

        # If new calculated z-block is righter than previous one
        if z_func[rightest_block] + rightest_block < z_func[i] + i:
            rightest_block = i

    z_func[0] = len(s)

    return z_func


def calculate_dummy_z_func(s):
    z = [0] * len(s)
    for i in range(len(s)):
        z[i] = get_longest_common_prefix(s, s[i:])

    return z


if __name__ == "__main__":
    while True:
        s = input()
        z_func = calculate_z_function(s)
        z_func_dummy = calculate_dummy_z_func(s)
        # if z_func != z_func_dummy:
        #     print(f"true = {z_func_dummy}")
        #     print(f"Me =   {z_func}")
        print(f"Me =   {z_func}")
