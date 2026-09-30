"""Real AT-SPI client of the standalone host bridge fixture, on a session bus."""
import subprocess,sys,time
import pyatspi
process=subprocess.Popen([sys.argv[1]],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
def descendants(node):
    yield node
    for child in node:
        yield from descendants(child)
try:
    limit=time.monotonic()+15
    app=None
    while time.monotonic()<limit and process.poll() is None:
        for candidate in pyatspi.Registry.getDesktop(0):
            if candidate.name=='Plan accessibility fixture':app=candidate;break
        if app:break
        time.sleep(.1)
    assert app,'application registered on accessibility bus'
    nodes={node.name:node for node in descendants(app)}
    action=nodes['Draw mark'].queryAction()
    presses=[i for i in range(action.nActions) if action.getName(i)=='press']
    assert presses and action.doAction(presses[0]),'button action succeeds'
    value=nodes['Canvas X'].queryValue()
    assert value.minimumValue==0 and value.maximumValue==200,'numeric range exported'
    value.currentValue=37
    assert nodes['Artwork text'].queryEditableText().setTextContents('مرحبا ABC'),'Unicode text edit succeeds'
    stdout,stderr=process.communicate(timeout=10)
    print(stdout,end='')
    assert process.returncode==0,stderr
    assert 'CRITICAL' not in stderr and 'WARNING' not in stderr,stderr
finally:
    if process.poll() is None:process.terminate();process.wait(timeout=5)
