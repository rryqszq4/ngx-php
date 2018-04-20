# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: declared classes
declared classes
--- config
location = /declared_classes {
    content_by_php '
        $class = get_declared_classes();
        foreach ($class as $v){
        	if ($v != "HashContext" && $v != "ArgumentCountError" && 
        	$v != "ReflectionNamedType" && 
        	$v != "ReflectionClassConstant"){
        		echo "{$v}\n";
        	}
        }
    ';
}
--- request
GET /declared_classes
--- response_body
stdClass
Exception
ErrorException
Error
ParseError
TypeError
ArithmeticError
DivisionByZeroError
Closure
Generator
ClosedGeneratorException
DateTime
DateTimeImmutable
DateTimeZone
DateInterval
DatePeriod
LibXMLError
SQLite3
SQLite3Stmt
SQLite3Result
CURLFile
DOMException
DOMStringList
DOMNameList
DOMImplementationList
DOMImplementationSource
DOMImplementation
DOMNode
DOMNameSpaceNode
DOMDocumentFragment
DOMDocument
DOMNodeList
DOMNamedNodeMap
DOMCharacterData
DOMAttr
DOMElement
DOMText
DOMComment
DOMTypeinfo
DOMUserDataHandler
DOMDomError
DOMErrorHandler
DOMLocator
DOMConfiguration
DOMCdataSection
DOMDocumentType
DOMNotation
DOMEntity
DOMEntityReference
DOMProcessingInstruction
DOMStringExtend
DOMXPath
finfo
LogicException
BadFunctionCallException
BadMethodCallException
DomainException
InvalidArgumentException
LengthException
OutOfRangeException
RuntimeException
OutOfBoundsException
OverflowException
RangeException
UnderflowException
UnexpectedValueException
RecursiveIteratorIterator
IteratorIterator
FilterIterator
RecursiveFilterIterator
CallbackFilterIterator
RecursiveCallbackFilterIterator
ParentIterator
LimitIterator
CachingIterator
RecursiveCachingIterator
NoRewindIterator
AppendIterator
InfiniteIterator
RegexIterator
RecursiveRegexIterator
EmptyIterator
RecursiveTreeIterator
ArrayObject
ArrayIterator
RecursiveArrayIterator
SplFileInfo
DirectoryIterator
FilesystemIterator
RecursiveDirectoryIterator
GlobIterator
SplFileObject
SplTempFileObject
SplDoublyLinkedList
SplQueue
SplStack
SplHeap
SplMinHeap
SplMaxHeap
SplPriorityQueue
SplFixedArray
SplObjectStorage
MultipleIterator
SessionHandler
__PHP_Incomplete_Class
php_user_filter
Directory
AssertionError
PDOException
PDO
PDOStatement
PDORow
PharException
Phar
PharData
PharFileInfo
ReflectionException
Reflection
ReflectionFunctionAbstract
ReflectionFunction
ReflectionGenerator
ReflectionParameter
ReflectionType
ReflectionMethod
ReflectionClass
ReflectionObject
ReflectionProperty
ReflectionExtension
ReflectionZendExtension
SimpleXMLElement
SimpleXMLIterator
SoapClient
SoapVar
SoapServer
SoapFault
SoapParam
SoapHeader
mysqli_sql_exception
mysqli_driver
mysqli
mysqli_warning
mysqli_result
mysqli_stmt
XMLReader
XMLWriter
ZipArchive
ngx
ngx_log
ngx_request
ngx_socket
ngx_var
