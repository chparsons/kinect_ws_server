#include "testApp.h"

void testApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetFrameRate(60);

	//TODO setup fron config.xml
	//near/far mm
	//kinect resize factor
	//port
	//document root
	
	out_resize_factor = 1.;
	init_out_pix();	

	kinect.setRegistration(false);
	kinect.setDepthClipping(500, 4000); //mm (50cm - 4mt)
	kinect.enableDepthNearValueWhite(true);

	// ir:false, rgb:false, texture:true
	kinect.init(false, false, true);
	kinect.open();

	ofxLibwebsockets::ServerOptions opt = 
		ofxLibwebsockets::defaultServerOptions();
	opt.port = 9093;
	opt.protocol = "of-protocol";
	opt.documentRoot = ofToDataPath("web/app_three", true); 
	bool connected = server.setup( opt );

	// this adds your app as a listener for the server
	server.addListener(this);

	ofLog(OF_LOG_NOTICE, 
		"WebSocket server setup at " + 
		ofToString( server.getPort() ) + 
		( server.usingSSL() ? " with SSL" : " without SSL") 
		);
}

void testApp::update()
{
	ofBackground(0);

	kinect.update();	

	if ( ! kinect.isFrameNew() )
		return;

	uint16_t *raw_depth_pix = kinect.getRawDepthPixels();
	ofPixels depth_pix = kinect.getDepthPixelsRef();
	depth_pix.resizeTo( out_pix );

	server.send(
		ofToString( out_pix.getWidth() )
		+":"+ 
		ofToString( out_pix.getHeight() )
		+":"+ 
		ofToString( out_pix.getNumChannels() ) 
		+":"+ 
		ofToString( out_pix.size() ) 
		);

	server.sendBinary( out_pix.getPixels(), out_pix.size() );
}

void testApp::draw()
{
	ofSetColor(255);
	kinect.drawDepth(0,0);

	//instructions
	stringstream str;
	str << "out_resize_factor: " << out_resize_factor 
		<< " / " << out_pix.getWidth() << " x " << out_pix.getHeight()
		<< " / fps " << ofGetFrameRate()
		<< endl;
	ofDrawBitmapString(str.str(), 10, 10);
}

void testApp::exit() 
{
	out_pix.clear();
	kinect.close();
}

void testApp::init_out_pix()
{
	out_pix.allocate( kinect.getWidth() * out_resize_factor, 
			kinect.getHeight() * out_resize_factor, 
			1); 
	out_pix.set(0);
}

void testApp::onConnect( ofxLibwebsockets::Event& args )
{
	ofLog(OF_LOG_NOTICE, " ### on connected");
}

void testApp::onOpen( ofxLibwebsockets::Event& args )
{
	ofLog(OF_LOG_NOTICE, " ### on open /new connection open from " 
			+ args.conn.getClientIP() );
}

void testApp::onClose( ofxLibwebsockets::Event& args )
{
	ofLog(OF_LOG_NOTICE, " ### on close");
}

void testApp::onIdle( ofxLibwebsockets::Event& args )
{
	ofLog(OF_LOG_NOTICE, " ### on idle");
}

void testApp::onMessage( ofxLibwebsockets::Event& args )
{
	ofLog(OF_LOG_NOTICE, " ### got message");

	// trace out string messages or JSON messages!
	if ( args.json != NULL)
	{
		ofLog(OF_LOG_NOTICE, "new message: " 
			+ args.json.toStyledString() 
			+ " from " 
			+ args.conn.getClientName() 
			);
	}
	else 
	{
		ofLog(OF_LOG_NOTICE, "new message: " 
			+ args.message 
			+ " from " 
			+ args.conn.getClientName() 
			);
	}

	// echo server = send message right back!
	//args.conn.send( args.message );
}

void testApp::onBroadcast( ofxLibwebsockets::Event& args )
{
	ofLog(OF_LOG_NOTICE, " ### got broadcast " + args.message);    
}

void testApp::keyPressed(int key)
{
}

void testApp::keyReleased(int key)
{
	float delta = 0.1;
	switch (key) 
	{
		case OF_KEY_UP:
			out_resize_factor += delta;
			out_resize_factor = CLAMP(out_resize_factor, 0., 1.);
			init_out_pix();
			break;
		case OF_KEY_DOWN:
			out_resize_factor -= delta;
			out_resize_factor = CLAMP(out_resize_factor, 0., 1.);
			init_out_pix();
			break;
	}
}

void testApp::mouseMoved(int x, int y )
{
}

void testApp::mouseDragged(int x, int y, int button)
{
}

void testApp::mousePressed(int x, int y, int button)
{
	string url = "http";
	if ( server.usingSSL() )
	{
		url += "s";
	}
	url += "://localhost:" + ofToString( server.getPort() );
	ofLaunchBrowser(url);
}

void testApp::mouseReleased(int x, int y, int button){}
void testApp::windowResized(int w, int h){}
void testApp::gotMessage(ofMessage msg){}
void testApp::dragEvent(ofDragInfo dragInfo){}
