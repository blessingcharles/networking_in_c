##############  TCP PACKETS FLOW (FTP) ##############
#create a simulator object
set ns [new Simulator]
#create a trace file and animation file
set tracefile [open ftp.tr w]
$ns trace-all $tracefile 
set namfile [open ftp.nam w]
$ns namtrace-all $namfile

#creating 5 nodes 
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]
set n4 [$ns node]
set n5 [$ns node]

$ns duplex-link $n0 $n1 5Mb 2ms DropTail
$ns duplex-link $n2 $n1 10Mb 4ms DropTail
$ns duplex-link $n1 $n4 3Mb 10ms DropTail
$ns duplex-link $n4 $n3 100Mb 2ms DropTail
$ns duplex-link $n4 $n5 4Mb 10ms DropTail

#creation of tcp agent from n2 to n5
set tcp [new Agent/TCP]
set sink [new Agent/TCPSink]
$ns attach-agent $n2 $tcp 
$ns attach-agent $n5 $sink 
$ns connect $tcp $sink

#setting color
$ns color 1 Red
$tcp set fid_ 1

#creation of application ftp(tcp) in local lan
set ftp [new Application/FTP]
$ftp attach-agent $tcp
#start the traffic
$ns at 2.0 "$ftp start"
$ns at 10.0 "finish" 

proc finish { } {
    global ns tracefile namfile
    $ns flush-trace
    close $tracefile
    close $namfile
    exit 0
}

puts "sivamesan network start"

$ns run