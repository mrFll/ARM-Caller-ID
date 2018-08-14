i pg_header.inc
t <h1>Relay codes</h1>
t <form action="codes.cgi" method="post" id="form1" name="form1">
t <table>
t <tr>
t <td><h3>Relay 1</h3></td>
t <td><h3>Relay 2</h3></td>
t <td><h3>Relay 3</h3></td>
t <td><h3>Relay 4</h3></td>
t </tr>	
t <tr>
t <td>*132#</td>
t <td>98**1</td>
t <td>#123*</td>
t <td>*7312</td>
t </tr>			
t <tr>
t <td><button name=setrel1 OnClick="submit();">invert</button></td>
t <td><button name=setrel2 OnClick="submit();">invert</button></td>
t <td><button name=setrel3 OnClick="submit();">invert</button></td>
t <td><button name=setrel4 OnClick="submit();">invert</button></td>
t </tr>			
t <tr>
t <td>
t <input type="text" name=rname1/>
t <button name=rsub1 OnClick="submit();">Submit</button>
t </td>
t <td>
t <input type="text"/>
t <input type="submit"/>
t </td>			
t <td>
t <input type="text"/>
t <input type="submit"/>
t </td>			
t <td>
t <input type="text"/>
t <input type="submit"/>
t </td>
t </tr>			
t </table>
t </form>
i pg_footer.inc
.
