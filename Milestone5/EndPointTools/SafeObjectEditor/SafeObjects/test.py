import pandas as pd

def test(*args):
    df = pd.DataFrame([[1, 2], [4, 5], [7, 8]], index=['cobra', 'viper', 'sidewinder'], columns=['max_speed', 'shield'])
    print(df)
    return df.loc[args]

print(test('viper'))
print(test(['viper', 'sidewinder']))
print(test(lambda df: df['shield'] == 8))


