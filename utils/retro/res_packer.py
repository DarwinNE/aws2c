import sys
import io
import struct

def pad_buffer(buffer: io.BytesIO, alignment = 256):
    cur_size = buffer.getbuffer().nbytes
    remainder = cur_size % alignment

    if remainder > 0:
        buffer.write(b'\00' * (alignment - remainder))

def round_header_entries(n):
    # Round up to the nearest higher 64 multiple
    return (63 + n) & ~63;

def convert_to_bin(input_path, output_path):
    try:
        data_buffer = io.BytesIO()
        header_buffer = io.BytesIO()
        current_id = 0

        with open(input_path, 'r', encoding='ASCII') as infile, open(output_path, 'wb') as outfile:
            offset = 0
            entries = 0

            # count the number of valid entries we have
            for line_num, line in enumerate(infile, 0):
                line = line.strip()
                if not line:
                    continue
                    
                entries = entries + 1
                
            entries = round_header_entries(entries)
            offset = entries * 4    # We have 4 bytes for every entry, so we want to make sure the offset is relative to the beginning of the file
            
            infile.seek(0) # Seek back at the beginning

            for line_num, line in enumerate(infile, 0):
                line = line.strip()
                if not line:
                    continue

                name = ""

                try:
                    # Split by space to separate the name from the hex values
                    # Format: "name value1,value2,value3"
                    parts = line.split(' ', 1)
                    if len(parts) < 2:
                        print(f"Skipping line {line_num}: No hex data found.")
                        continue

                    name, hex_content = parts

                    # Clean the hex string and generate a binary array
                    hex_parts = hex_content.replace('0x','').split(',')
                    hex_parts = [h.zfill(2) for h in hex_parts]
                    hex_string = ''.join(hex_parts)
                    binary_data = bytes.fromhex(hex_string)

                    # Write data to the buffer
                    data_buffer.write(binary_data)

                    # update header data, write offset and length as 16-bit LE numbers
                    header_buffer.write(struct.pack('<HH', offset, len(binary_data)))

                    # Update the offset that the data occupies in the in the file
                    offset = offset + len(binary_data)

                    # Increment the current ID
                    current_id = current_id + 1
                except ValueError as e:
                    print(f"Error on line {line_num} ('{name}'): {e}")

            pad_buffer(header_buffer, 256)
            pad_buffer(data_buffer, 256)

            # Write the content of the buffers to the file
            outfile.write(header_buffer.getvalue())
            outfile.write(data_buffer.getvalue())
            
        print(f"Binary file saved to: {output_path}")

    except FileNotFoundError:
        print(f"Error: The file '{input_path}' was not found.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    # Usage: python script.py input_text_resources binary_output
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_file> <output_file>")
    else:
        convert_to_bin(sys.argv[1], sys.argv[2])
