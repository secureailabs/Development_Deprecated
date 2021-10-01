
print(__data)
if(__data.size<=5):
    print("privacy violate")
    exit(123)

__result = __data.mean(axis=__axis, skipna=__skipna, level=__level, numeric_only=__numeric_only, **__kwargs)
