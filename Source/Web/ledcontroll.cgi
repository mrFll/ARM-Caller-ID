i pg_header.inc
t <p> check and uncheck to turn on and off the leds on the board</p>
t <form action=ledcontroll.cgi method=POST name=form1>
c l 0 <input type=checkbox name=led0 OnClick="submit();" %s>1
c l 1 <input type=checkbox name=led1 OnClick="submit();" %s>2
c l 2 <input type=checkbox name=led2 OnClick="submit();" %s>3
c l 3 <input type=checkbox name=led3 OnClick="submit();" %s>4
t <br> keyListener:<input type="checkbox" id="refreshChkBox" onclick="periodicUpdate()">
t <br>
t </form>
i pg_footer.inc
.
