/****************************************************************************
 * CSVTranspose.cpp                                                         *
 * Written by Patrick Reilly                                                *
 * Version 1.0 written 2015/06/17                                           *
 * Description:                                                             *
 *  This program transposes basic CSV files (no commas or newlines within   *
 *  elements).  The original purpose was to convert wide ancestry call CSV  *
 *  files from MSG into tall CSVr files for faster import into R/qtl.       *
 *                                                                          *
 * Syntax: CSVTranspose input_CSV_file output_CSV_file                      *
 *  input_CSV_file:  Path to the wide CSV file                              *
 *  output_CSV_file: Path and name for the output tall CSV file.            *
 *                    The file will be overwritten if it already exists.    *
 *                                                                          *
 * Design:                                                                  *
 *  The general idea was suggested by Jim Baldwin-Brown, implemented in Perl*
 *  by Patrick Reilly, and due to memory usage concerns, reimplemented here.*
 *  The first row of the input CSV is read, chomped, and each element is    *
 *  added to a queue.  For each subsequent row, the line is chomped, and    *
 *  tokenized while concatenating each token to the associated string in the*
 *  output queue.  In this manner, the output CSV is progressively built    *
 *  line-by-line within the queue, and then each line is output to the      *
 *  output CSV file.                                                        *
 *  The maximum memory usage in this script should be close to the size of  *
 *  the input file plus the size of the last row of the input file.         *
 ****************************************************************************/

#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <cstdio>
#include <cstdlib>

using namespace std;

int main(int argc, const char **argv) {
   ifstream input_CSV;
   ofstream output_CSV;
   string input_row, temp_output;
   char curchar;
   queue<string> output_rows;
   size_t queue_size;
   size_t row_size;
   string usage = "Usage: " + string(argv[0]) + " input_CSV_file output_CSV_file";
   string element = "";
   if (argc < 3) {
      cerr << "Not enough arguments." << endl << usage << endl;
      return 1;
   }
   input_CSV.open(argv[1]);
   if (!input_CSV) {
      cerr << "Error opening input CSV file." << endl;
      return 2;
   }
   //Read in the first row of the input file:
   getline(input_CSV, input_row);
   //Check for stream errors:
   if (!input_CSV) {
      cerr << "Error reading input CSV file." << endl;
      return 3;
   }
   //Split the first row based on commas, and store each element in the queue:
   for (size_t i = 0; i < input_row.size(); i++) {
      curchar = input_row[i];
      if (i == input_row.size()-1) { //Store the last element
         element += curchar;
         output_rows.push(element);
         element = "";
         break;
      } else if (curchar == ',') { //Store any but the last element
         output_rows.push(element);
         element = "";
      } else { //Build the token
         element += curchar;
      }
   }
   //Determine the queue size for checking the validity of the input CSV file:
   //That is, make sure the input file has a constant number of columns
   queue_size = output_rows.size();
   //Iterate over the remaining rows of the input file:
   //Read in the second row:
   getline(input_CSV, input_row);
   while (input_CSV.good() || !input_CSV.fail()) {
      row_size = 0;
      //Split the row based on commas, and append each element to its associated queue element:
      for (size_t i = 0; i < input_row.size(); i++) {
         curchar = input_row[i];
         if (output_rows.empty()) {
            cerr << "Empty queue error." << endl;
            return 4;
         }
         if (i == input_row.size()-1) { //Append to the last row
            element += curchar;
            temp_output = output_rows.front();
            output_rows.pop();
            temp_output += ',' + element;
            output_rows.push(temp_output);
            element = "";
            row_size++;
            if (row_size != queue_size) {
               cerr << "Malformatted CSV file: Different numbers of columns per row." << endl;
               return 5;
            }
            break;
         } else if (curchar == ',') { //Append to any but the last row
            temp_output = output_rows.front();
            output_rows.pop();
            temp_output += ',' + element;
            output_rows.push(temp_output);
            element = "";
            row_size++;
         } else { //Build the token
            element += curchar;
         }
      }
      //Read in the next row if not already at EOF:
      if (!input_CSV.eof()) {
         getline(input_CSV, input_row);
      } else {
         break;
      }
   }
   //Check for stream errors:
   if (input_CSV.eof() != true) {
      cerr << "Error reading input CSV file." << endl;
      return 3;
   }
   //Close the input file:
   input_CSV.close();
   //Output the transposed CSV file:
   //Open the output file:
   output_CSV.open(argv[2], fstream::trunc);
   //Check for errors:
   if (!output_CSV) {
      cerr << "Error opening output CSV file." << endl;
      return 6;
   }
   //Loop over the rows:
   while (!output_rows.empty() && output_CSV.good()) {
      //Get a row from the queue:
      temp_output = output_rows.front();
      output_rows.pop();
      output_CSV << temp_output << endl;
   }
   //Close the output file:
   output_CSV.close();
   return 0;
}
