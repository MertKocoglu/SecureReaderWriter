
---

# Reader-Writer Problem with Password Authentication in C

## Overview

This project implements a solution to the classic **Reader-Writer problem** using **POSIX threads (pthread)** and **semaphores** on macOS/Linux platforms.
The program enforces **password-based authentication** where each thread must register a unique password (thread hash) to access a shared resource (`BUFFER`). Additionally, dummy threads are created that do not authenticate but also access the resource. The program ensures proper synchronization for concurrent readers and exclusive access for writers.

---

## Features

* Multi-threading with pthreads (real and dummy reader/writer threads)
* Password authentication using thread hash values
* Semaphore-based synchronization using macOS Grand Central Dispatch (`dispatch_semaphore_t`)
* Readers can access the shared resource concurrently
* Writers have exclusive access to the shared resource
* Supports multiple test cases with different reader/writer counts
* Thread-safe password table management with mutex protection
* Outputs all access logs in tabular form to a file `output.txt`
* Error handling for thread creation and file operations

---

## Files

* `reader_writer.c` : Main source code implementing the solution
* `output.txt` : Output file generated after running the program showing access logs for all test cases

---

## Requirements

* GCC or Clang compiler
* POSIX-compliant OS (macOS, Linux)
* pthread and Grand Central Dispatch support (macOS built-in)

---

## Compilation

Use the following command to compile the program:

```bash
gcc -o reader_writer reader_writer.c -pthread
```

> Note: Using `-pthread` ensures proper multithreading support during compilation and linking.

---

## Usage

Run the compiled executable:

```bash
./reader_writer
```

This will run three test cases with different numbers of reader and writer threads. The program creates equal numbers of dummy threads as well. Each thread performs 5 read/write operations with appropriate synchronization.

---

## Output

* The program writes detailed logs of each access (real and dummy threads) to `output.txt`.
* The log includes:

  * Thread number
  * Thread hash (password)
  * Validity (`real` or `dummy`)
  * Role (`reader` or `writer`)
  * Value read or written to the shared BUFFER

---

## Example of Output (excerpt)

```

--- Test Case 1 ---
Thread_No  Hash_Value      Validity   Role       Value read/written
---------------------------------------------------------------
1          6162444288      real       reader     0              
2          6163017728      real       reader     0              
3          6163591168      real       reader     0              
1          6165311488      dummy      reader     0              
2          6165884928      dummy      reader     0              
3          6166458368      dummy      reader     0              
1          6164164608      real       writer     3893           
2          6164738048      real       writer     1139           
1          6167031808      dummy      writer     461            
2          6167605248      dummy      writer     3928           
1          6162444288      real       reader     3928           
2          6165884928      dummy      reader     3928           
1          6165311488      dummy      reader     3928           
3          6166458368      dummy      reader     3928           
2          6163017728      real       reader     3928           
3          6163591168      real       reader     3928           
1          6164164608      real       writer     1902           
2          6164738048      real       writer     1761           
1          6167031808      dummy      writer     5393           
2          6167605248      dummy      writer     2335           
2          6165884928      dummy      reader     2335           
3          6163591168      real       reader     2335           
2          6163017728      real       reader     2335           
3          6166458368      dummy      reader     2335           
1          6162444288      real       reader     2335           
1          6165311488      dummy      reader     2335           
1          6164164608      real       writer     6887           
2          6164738048      real       writer     2983           
1          6167031808      dummy      writer     817            
2          6167605248      dummy      writer     6782           
3          6166458368      dummy      reader     6782           
1          6165311488      dummy      reader     6782           
2          6163017728      real       reader     6782           
3          6163591168      real       reader     6782           
2          6165884928      dummy      reader     6782           
1          6162444288      real       reader     6782           
1          6164164608      real       writer     8573           
2          6164738048      real       writer     9931           
1          6167031808      dummy      writer     3179           
2          6167605248      dummy      writer     3930           
3          6166458368      dummy      reader     3930           
1          6162444288      real       reader     3930           
3          6163591168      real       reader     3930           
2          6163017728      real       reader     3930           
2          6165884928      dummy      reader     3930           
1          6165311488      dummy      reader     3930           
1          6164164608      real       writer     3240           
2          6164738048      real       writer     584            
1          6167031808      dummy      writer     2846           
2          6167605248      dummy      writer     1242           

--- Test Case 2 ---
Thread_No  Hash_Value      Validity   Role       Value read/written
---------------------------------------------------------------
1          6162444288      real       reader     0              
2          6163017728      real       reader     0              
3          6163591168      real       reader     0              
4          6164164608      real       reader     0              
5          6164738048      real       reader     0              
1          6168178688      dummy      reader     0              
2          6168752128      dummy      reader     0              
3          6169325568      dummy      reader     0              
4          6169899008      dummy      reader     0              
5          6170472448      dummy      reader     0              
1          6165311488      real       writer     7376           
2          6165884928      real       writer     9148           
3          6166458368      real       writer     2400           
4          6167031808      real       writer     7882           
5          6167605248      real       writer     9265           
1          6171045888      dummy      writer     9962           
2          6171619328      dummy      writer     410            
3          6172192768      dummy      writer     1926           
4          6172766208      dummy      writer     5424           
5          6173339648      dummy      writer     6195           
3          6163591168      real       reader     6195           
1          6162444288      real       reader     6195           
5          6170472448      dummy      reader     6195           
3          6169325568      dummy      reader     6195           
5          6164738048      real       reader     6195           
4          6164164608      real       reader     6195           
2          6163017728      real       reader     6195           
2          6168752128      dummy      reader     6195           
4          6169899008      dummy      reader     6195           
1          6168178688      dummy      reader     6195           
1          6165311488      real       writer     5495           
2          6165884928      real       writer     7631           
3          6166458368      real       writer     3695           
4          6167031808      real       writer     3926           
5          6167605248      real       writer     5003           
1          6171045888      dummy      writer     2260           
2          6171619328      dummy      writer     4943           
3          6172192768      dummy      writer     7190           
4          6172766208      dummy      writer     1190           
5          6173339648      dummy      writer     6139           
5          6170472448      dummy      reader     6139           
3          6169325568      dummy      reader     6139           
2          6168752128      dummy      reader     6139           
5          6164738048      real       reader     6139           
1          6162444288      real       reader     6139           
1          6168178688      dummy      reader     6139           
4          6164164608      real       reader     6139           
3          6163591168      real       reader     6139           
4          6169899008      dummy      reader     6139           
2          6163017728      real       reader     6139           
1          6165311488      real       writer     6796           
2          6165884928      real       writer     9629           
3          6166458368      real       writer     8248           
4          6167031808      real       writer     3610           
5          6167605248      real       writer     3533           
1          6171045888      dummy      writer     2392           
2          6171619328      dummy      writer     7930           
3          6172192768      dummy      writer     2668           
4          6172766208      dummy      writer     3918           
5          6173339648      dummy      writer     5489           
5          6164738048      real       reader     5489           
1          6162444288      real       reader     5489           
3          6169325568      dummy      reader     5489           
5          6170472448      dummy      reader     5489           
2          6163017728      real       reader     5489           
4          6169899008      dummy      reader     5489           
4          6164164608      real       reader     5489           
3          6163591168      real       reader     5489           
1          6168178688      dummy      reader     5489           
2          6168752128      dummy      reader     5489           
1          6165311488      real       writer     970            
2          6165884928      real       writer     8563           
3          6166458368      real       writer     8339           
4          6167031808      real       writer     3298           
5          6167605248      real       writer     1078           
1          6171045888      dummy      writer     9530           
2          6171619328      dummy      writer     78             
3          6172192768      dummy      writer     3460           
4          6172766208      dummy      writer     3577           
5          6173339648      dummy      writer     8979           
2          6168752128      dummy      reader     8979           
1          6162444288      real       reader     8979           
4          6169899008      dummy      reader     8979           
3          6163591168      real       reader     8979           
2          6163017728      real       reader     8979           
5          6170472448      dummy      reader     8979           
1          6168178688      dummy      reader     8979           
4          6164164608      real       reader     8979           
3          6169325568      dummy      reader     8979           
5          6164738048      real       reader     8979           
1          6165311488      real       writer     8637           
2          6165884928      real       writer     5442           
3          6166458368      real       writer     6143           
4          6167031808      real       writer     1423           
5          6167605248      real       writer     7644           
1          6171045888      dummy      writer     7035           
2          6171619328      dummy      writer     1734           
3          6172192768      dummy      writer     1512           
4          6172766208      dummy      writer     5295           
5          6173339648      dummy      writer     5468           

--- Test Case 3 ---
Thread_No  Hash_Value      Validity   Role       Value read/written
---------------------------------------------------------------
1          6162444288      real       reader     0              
1          6167605248      dummy      reader     0              
1          6163017728      real       writer     9830           
2          6163591168      real       writer     5891           
3          6164164608      real       writer     3764           
4          6164738048      real       writer     4190           
5          6165311488      real       writer     9506           
6          6165884928      real       writer     1977           
7          6166458368      real       writer     6718           
8          6167031808      real       writer     2278           
1          6168178688      dummy      writer     7589           
2          6168752128      dummy      writer     4983           
3          6169325568      dummy      writer     1488           
4          6169899008      dummy      writer     6441           
5          6170472448      dummy      writer     3877           
6          6171045888      dummy      writer     4186           
7          6171619328      dummy      writer     3333           
8          6172192768      dummy      writer     7010           
1          6167605248      dummy      reader     7010           
1          6162444288      real       reader     7010           
1          6163017728      real       writer     8503           
2          6163591168      real       writer     3341           
3          6164164608      real       writer     9872           
4          6164738048      real       writer     7475           
5          6165311488      real       writer     2100           
6          6165884928      real       writer     3540           
7          6166458368      real       writer     1742           
8          6167031808      real       writer     3814           
1          6168178688      dummy      writer     728            
2          6168752128      dummy      writer     7833           
3          6169325568      dummy      writer     9038           
4          6169899008      dummy      writer     9218           
5          6170472448      dummy      writer     1101           
6          6171045888      dummy      writer     8605           
7          6171619328      dummy      writer     5050           
8          6172192768      dummy      writer     2302           
1          6167605248      dummy      reader     2302           
1          6162444288      real       reader     2302           
1          6163017728      real       writer     761            
2          6163591168      real       writer     2719           
3          6164164608      real       writer     4788           
4          6164738048      real       writer     1248           
5          6165311488      real       writer     8258           
6          6165884928      real       writer     420            
7          6166458368      real       writer     9509           
8          6167031808      real       writer     6212           
1          6168178688      dummy      writer     1559           
2          6168752128      dummy      writer     7856           
3          6169325568      dummy      writer     2081           
4          6169899008      dummy      writer     9662           
5          6170472448      dummy      writer     4249           
6          6171045888      dummy      writer     1212           
7          6171619328      dummy      writer     3344           
8          6172192768      dummy      writer     7269           
1          6162444288      real       reader     7269           
1          6167605248      dummy      reader     7269           
1          6163017728      real       writer     6466           
2          6163591168      real       writer     4284           
3          6164164608      real       writer     4761           
4          6164738048      real       writer     9662           
5          6165311488      real       writer     185            
6          6165884928      real       writer     659            
7          6166458368      real       writer     2552           
8          6167031808      real       writer     9277           
1          6168178688      dummy      writer     3604           
2          6168752128      dummy      writer     803            
3          6169325568      dummy      writer     2806           
4          6169899008      dummy      writer     8770           
5          6170472448      dummy      writer     8785           
6          6171045888      dummy      writer     394            
7          6171619328      dummy      writer     7207           
8          6172192768      dummy      writer     6960           
1          6162444288      real       reader     6960           
1          6167605248      dummy      reader     6960           
1          6163017728      real       writer     3118           
2          6163591168      real       writer     7451           
3          6164164608      real       writer     8284           
4          6164738048      real       writer     3076           
5          6165311488      real       writer     5873           
6          6165884928      real       writer     9305           
7          6166458368      real       writer     8814           
8          6167031808      real       writer     5697           
1          6168178688      dummy      writer     797            
2          6168752128      dummy      writer     8035           
3          6169325568      dummy      writer     9706           
4          6169899008      dummy      writer     3343           
5          6170472448      dummy      writer     2424           
6          6171045888      dummy      writer     6768           
7          6171619328      dummy      writer     3199           
8          6172192768      dummy      writer     6863           


---

## Notes

* The program uses `dispatch_semaphore_t` for synchronization on macOS to avoid deprecated POSIX semaphore warnings.
* Password authentication prevents unauthorized access to the shared resource by real threads. Dummy threads bypass authentication as required.
* Thread creation errors are handled gracefully with error messages.
* Sleep delays simulate operation durations and improve output readability.

---

## Author

Mert Koçoğlu- mertkocoglu3@gmail.com

---

# SecureReaderWriter
