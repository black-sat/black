import pandas as pd

# Define the file paths for your two CSV files
file1_path = 'future-220916.dat'
file2_path = 'ls4-230928.dat'

# Read the CSV files with space as the separator
df1 = pd.read_csv(file1_path, sep=' ')
df2 = pd.read_csv(file2_path, sep=' ')

# Remove old ls4 results
columns_to_drop = ['ls4/default', 'ls4/default:result']
df1.drop(columns=columns_to_drop, inplace=True)

# List of common columns for the join
common_columns = ['formula', 'type', 'family']

# Merge the two dataframes based on multiple common columns
merged_df = pd.merge(df1, df2, on=common_columns)

# Reset the index if needed
# merged_df.reset_index(drop=True, inplace=True)

# Save the merged dataframe to a new CSV file
merged_df.to_csv('merged_ls4-230928_into_future-230928.csv', sep=' ', index=False)

# Print a sample of the merged dataframe
print(merged_df.head())
