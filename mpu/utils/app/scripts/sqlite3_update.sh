#!/bin/bash

# Define paths for version file and database (easy for later maintenance)
VERSION_FILE="/oemapp/upgrade/version_uds.txt"
DB_PATH="/oemdata/parameters/paramsDb"

# Check if the version file exists
if [ ! -f "$VERSION_FILE" ]; then
    echo "Error: Version file $VERSION_FILE does not exist"
    exit 1
fi

# Read content from version file and remove whitespace (e.g., newlines, spaces)
version_content=$(cat "$VERSION_FILE" | tr -d '[:space:]')

# Check if the version file content is empty
if [ -z "$version_content" ]; then
    echo "Error: Version file $VERSION_FILE has empty content"
    exit 1
fi

# Split content into an array using underscore as delimiter
IFS='_' read -ra parts <<< "$version_content"

# Check if the split result has at least 6 parts (matches expected format)
if [ ${#parts[@]} -lt 6 ]; then
    echo "Error: Version file content format is invalid. Expected at least 6 parts separated by underscores"
    exit 1
fi

# Extract target fields by array index
software_version="${parts[2]}"   # Software version (3rd part, index 2)
part_number="${parts[3]}"        # Part number (4th part, index 3)
supplier_code="${parts[4]}"      # Supplier code (5th part, index 4)

# Define database update function (simplify repeated update logic)
update_db() {
    local id=$1
    local value=$2
    sqlite3 "$DB_PATH" "update paramstbl set value='$value' where id=$id"
    # Check if the update operation succeeded
    if [ $? -ne 0 ]; then
        echo "Error: Failed to update database (ID=$id)"
        exit 1
    fi
}

# Execute database updates
echo "Starting database updates..."
update_db 208 "$software_version"   # Write software version to ID=208
# update_db 14 "$software_version"   # Write software version to ID=208
update_db 19 "$part_number"        # Write part number to ID=19
update_db 20 "$supplier_code"      # Write supplier code to ID=20
update_db 16 "20231215"            # Write fixed value to ID=16
# update_db 15 "2023121501"          # Write fixed value to ID=15

echo "All database updates completed successfully"
exit 0