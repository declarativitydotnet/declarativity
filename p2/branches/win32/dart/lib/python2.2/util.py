def gettext(nodelist):
    """Return string for text in nodelist (encode() for Unicode conversion)"""
    text = ""
    for node in nodelist:
        if node.nodeType == node.TEXT_NODE:
            text = text + node.data
    return text.strip().encode()
