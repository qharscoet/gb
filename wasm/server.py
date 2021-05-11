import http.server
from http.server import HTTPServer, BaseHTTPRequestHandler
import socketserver

PORT = 8080

Handler = http.server.SimpleHTTPRequestHandler
Handler.extensions_map['.wasm'] = 'application/wasm'
Handler.extensions_map['.js'] = 'application/javascript'

httpd = socketserver.TCPServer(("127.0.0.1", PORT), Handler)

print("serving at port",PORT)
httpd.serve_forever()