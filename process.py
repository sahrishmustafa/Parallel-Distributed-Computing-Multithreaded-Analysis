import os
import shutil

# Set source directory (parent folder containing year folders)
source_dir = "/home/sahrish/Desktop/Semester 6/words_1/data/multiUN.en/un/text/en"

# Set target directory where all .snt files will be moved
target_dir = "/home/sahrish/Desktop/Semester 6/words_1/data/multiUN.en/un/text/en/combined_folder"
os.makedirs(target_dir, exist_ok=True)  # Create target directory if it doesn't exist

# Iterate over year folders
for year_folder in os.listdir(source_dir):
    year_path = os.path.join(source_dir, year_folder)
    
    if os.path.isdir(year_path):  # Ensure it's a directory
        for file in os.listdir(year_path):
            if file.endswith(".snt"):  # Move only .snt files
                source_file = os.path.join(year_path, file)
                target_file = os.path.join(target_dir, file)
                
                # Ensure unique filenames if duplicate exists
                counter = 1
                while os.path.exists(target_file):
                    name, ext = os.path.splitext(file)
                    target_file = os.path.join(target_dir, f"{name}_{counter}{ext}")
                    counter += 1

                shutil.move(source_file, target_file)

print("All files moved successfully!")
