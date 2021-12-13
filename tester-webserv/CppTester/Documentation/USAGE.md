## Arguments
	./a.out [TAGS]
## Client Settings
	NONE
	TIMEOUT
		Close the connection after N (timeout) secconds of no activity.
		On my default.
	PIPELINED
		Send requests without waiting for each response.
	WAIT_FOR_CLOSE
		Wait N (wait_close) seconds after all requests have been sent for the server
		to close the connection. If the server closes the connection in this time,
		the client returns success.

### On by Default
- TIMEOUT


## Creating Testcases
	Create testcases as functions in the 'src/TestCases/Tests' directory.
	Any function following the prototype 'TestCase testCase...();' with '...' being anything
	will automatically be added as an executed testcase.