def bwt(s):
    """Apply Burrows-Wheeler transform to input string. Not indicated by a unique byte but use index list"""
    # Table of rotations of string
    table = [s[i:] + s[:i] for i in range(len(s))]
    # Sorted table
    table_sorted = table[:]
    table_sorted.sort()
    print(table_sorted)
    # Get index list of ((every string in sorted table)'s next string in unsorted table)'s index in sorted table
    indexlist = []
    for t in table_sorted:
        index1 = table.index(t)
        index1 = index1+1 if index1 < len(s)-1 else 0
        index2 = table_sorted.index(table[index1])
        indexlist.append(index2)
    # Join last characters of each row into string
    r = ''.join([row[-1] for row in table_sorted])
    return r, indexlist

def ibwt(r,indexlist):
    """Inverse Burrows-Wheeler transform. Not indicated by a unique byte but use index list"""
    s = ''
    x = indexlist[0]
    for _ in r:
        s = s + r[x]
        x = indexlist[x]
    return s