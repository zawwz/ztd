# ZFD format {#zfd}

ZFD is composed of infinitely concatenable "chunks". There a three types of chunks:  
- Map chunk     : between braces    {}  
- List chunk    : between brackets  []  
- String value

Formatting is ignored, all spaces will be ignored unless they are part of a value.  
Comments can be written with //, ends at end of line

ZFD treats everything as a string, there is no number or boolean types

### Map Chunk

A map chunk consists of pairs of keys and values. Format is as follows:
```
{
  //no concatenator: value ends at end of line. /!\ Comments on these lines will be part of the value
  key1=value
  //concatenators: value ends at end of concatenation
  key2 = " value "
  key3 = ' value '
  key4=[ //list ]
  key5={ //chunk }
}
```

### List Chunk  

A list chunk consists of a linear list of values separated by commas. Format is as follows:
```
[
  //comments cannot be written between a value and the separating comma
  value,
  " value ",
  ' value ',
  { //chunk },
  [ //list ]
]
```

# Usage

## Definition

```cpp
ztd::filedat file("/path/to/file");
```

## Importing and reading

### Importing

```cpp
ztd::filedat file;
file.import_file("path/to/file"); //import from file
file.import_stdin();              //import from stdin
file.import_string("{a=b}");      //read from argument string
```
Throws exceptions if errors are encountered

### Reading

#### Accessing chunks

```cpp
ztd::chunkdat& chk1 = file["string"];  //key "string"
ztd::chunkdat& chk2 = file[0];         //element #0
ztd::chunkdat& chk3 = file[0]["key"];  //key "key" in element 0
```

String keys for map chunks, int keys for list chunks  

#### Getting a string value

```cpp
ztd::chunkdat& chk;
std::string str=chk;
std::string str=chk.strval();
```
> String type casting is automatic in cases where it applies, but you can use ``chk.strval()`` where necessary

## Write and Export to file

### Writing

```cpp
ztd::chunkdat chk, chk2;

chk  = "{ a = b }";                   // Assigning as raw string
chk2 = "[]";

chk  += std::make_pair("foo", "bar"); // Adding values to map. Can give a vector for multiple elements
chk2 += "val";                        // Adding values to list. Can give a vector for multiple elements

chk["foo"] = "foo";                   // Changing map values
chk2[0]    = "foo";                   // Changing list values

chk  -= "foo";                        // Remove element from map
chk2 -= 0;                            // Remove element from list

chk  *= "{ key = val }";              // Concatenate map chunks
chk2 *= "[ v1 , v2 ]";                // Concatenate string chunks
                                      // Chunks have to be of the same type for concatenation
```

> General operator rules apply here. You can use these operators without direct assign (+ instead of +=) and daisy-chain them

> In case a chunk doesn't have a type, it will be automatically set to the type the operation implies

It is advised to first write the data onto a chunk and then assigning the chunk to the file
But in case you need absolute performance, you can access the filedat chunk with `filedat.data()`

### Exporting

#### File export

```cpp
ztd::chunkdat& chk;
ztd::filedat file("/path/to/file");
file = chk;
file.export_file("/path/to/file");
```

#### Other

```cpp
ztd::chunkdat& chk;
ztd::filedat& file;
std::cout << chk << std::endl;
std::cout << file << std::endl;
```

## Exception handling

All filedat and chunkdat functions throw exceptions when errors are encountered

### Definition

```cpp
ztd::format_error(std::string what, std::string origin, std::string data, int where);
```

### Example use

```cpp
ztd::filedat file("/path/to/file");
try
{
  file.importFile();
}
catch (ztd::format_error& fe)
{
  printFormatException(fe);
}
```
If origin is known, printFormatException will print only the relevant line with location  
If origin is unknown, printFormatException will print the whole data until the discriminating line
