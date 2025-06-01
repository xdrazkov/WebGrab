# WebGrab

WebGrab is a multi-threaded file downloading utility written in C++. It provides a simple command-line interface for downloading files from URLs.


## Usage

After starting WebGrab, you can use the following commands:

- `download <url>` or `dl <url>`: Add a URL to the processing queue
- `queue`: Display the current number of tasks in the queue
- `quit`: Exit the application (waits for pending tasks to complete)

## Example
```bash
> WebGrab.exe
WebGrab started.
> download https://example.com/sample.zip
Added download task for URL: https://example.com/sample.zip
> download https://example.com/image.jpg
Added download task for URL: https://example.com/image.jpg
> quit
Waiting for tasks to finish...
Finished processing URL: https://example.com/sample.zip
Finished processing URL: https://example.com/image.jpg
WebGrab terminated.
```
