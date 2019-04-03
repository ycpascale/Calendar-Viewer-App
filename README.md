This web app displays information about .ics file. For example: How many calendar properties, event components and alarm components
does the .ics file have.

Installation of Node App
1. Install
# From the root of the directory
npm install
2. Running Server
# PORT is your personally given port number, e.g. 1234
npm run dev PORT
# Server will be accessible at http://localhost:PORT
Directory Structure
# This contains the Backend Node Server, with our Web Application and API
app.js
# These are the package configuration files for npm to install dependencies
package.json
package-lock.json
# This is the Frontend HTML file that you see when you visit the document root
public/index.html
# This is the Frontend browser JavaScript file
public/index.js
# This is the Frontend Custom Style Sheet file
public/style.css
# This is the directory for uploaded .ics files
upload/
# This is the directory where you put all your C parser code
parser/

How does everything work together? (On School Server)
1. Install the dependencies (only need to do this once) and spin up our node
server
• Note: We’re using “nodemon” (instead of say node run dev) because
it hot-reloads app.js whenever it’s changed
2. View at http://localhost:PORT
3. The HTML is loaded when you visit the page and see forms, tables, content
4. The CSS is also loaded, and you’ll see the page has style
5. The JavaScript file is loaded (index.js) and will run a bunch of “on load”
AJAX calls to populate dropdowns, change elements
6. When buttons are clicked, more AJAX calls are made to the backend, that
recieve a response update the HTML
7. An AJAX call is made from your browser, it makes an HTTP (GET,
POST. . . ) call to our web server
8. The app.js web server receives the request with the route, and request data
(JSON, url parameters, files. . . )
9. Express looks for the route you defined, then runs the callback function
you provided
10. Our callback function (for this module) should just return a hard coded
JSON response
11. The AJAX call gets a response back from our server (either a 200
OK or maybe an error like a 404 not found) and either calls the
“success” callback function or the “fail” function. If the success is
called, “data” contains the returned JSON, and we can use it to update
elements, e.g.$('#elementId').html('<div>' + data["somekey"] +
'</div>'); where there is a “div” somewhere with the “id” “elementId”.
