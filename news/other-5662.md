stats-exporter(): applied changes arte:
- any internal request or response processing errors which cannot be responded with a valid HTTP response, will now log the error and close the connection
- the SCL module single-instance() option is synced correctly with the stats-exporter module's single-instance() option, so the default value must be `yes` in every case
- the response content-type is set according to the requested stat-format()
- added an internal chunked response solution, so large responses should not cause stalls anymore
- set the default scrape-freq-limit() to 15
