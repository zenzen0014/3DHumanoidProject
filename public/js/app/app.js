/**
 * @author mrdoob / http://mrdoob.com/
 */

var APP = {

	Player: function () {




		var loader = new THREE.ObjectLoader();
		var camera, scene, renderer;

		var controls, effect, cameraVR, isVR;

		var events = {};

		this.dom = document.createElement( 'div' );

		this.width = 500;
		this.height = 500;






		var url = window.location.href;
		var arr = url.split("/");
		var result = arr[0] + "//" + arr[2];
		var socket = io.connect(result);

		var id = new Date();













		this.load = function ( json ) {
			console.log("Accessed "+result);
			// isVR = json.project.vr;

			renderer = new THREE.WebGLRenderer( { antialias: true } );
			renderer.setClearColor( 0x000000 );
			renderer.setPixelRatio( window.devicePixelRatio );

			if ( json.project.gammaInput ) renderer.gammaInput = true;
			if ( json.project.gammaOutput ) renderer.gammaOutput = true;

			if ( json.project.shadows ) {

				renderer.shadowMap.enabled = true;
				// renderer.shadowMap.type = THREE.PCFSoftShadowMap;

			}

			this.dom.appendChild( renderer.domElement );






			this.setScene( loader.parse( json.scene ) );
			this.setCamera( loader.parse( json.camera ) );


			// // CAMERA

			// camera = new THREE.PerspectiveCamera( 45, window.innerWidth / window.innerHeight, 1, 4000 );
			// camera.position.set( 0, 150, 1300 );

			// // SCENE

			// scene = new THREE.Scene(json.scene);
			// scene.background = new THREE.Color( 0xffffff );
			// scene.fog = new THREE.Fog( 0xffffff, 1000, 4000 );

			// scene.add( camera );



			// LIGHTS

			scene.add( new THREE.AmbientLight( 0x222222 ) );

			var light = new THREE.DirectionalLight( 0xffffff, 1 ); //color , intensity
			light.position.set( 200, 200, 0 ); //This is set equal to Object3D.DefaultUp (0, 1, 0), so that the light shines from the top down.

			light.castShadow = true;

			light.shadow.mapSize.width = 1024;
			light.shadow.mapSize.height = 512;

			light.shadow.camera.near = 100;
			light.shadow.camera.far = 1200;

			light.shadow.camera.left = -1000;
			light.shadow.camera.right = 1000;
			light.shadow.camera.top = 350;
			light.shadow.camera.bottom = -350;

			scene.add( light );




			// var light2 = new THREE.DirectionalLight( 0xffffff, 1 ); //color , intensity
			// light2.position.set( -200, 200, 0 ); //This is set equal to Object3D.DefaultUp (0, 1, 0), so that the light shines from the top down.

			// light2.castShadow = true;

			// light2.shadow.mapSize.width = 1024;
			// light2.shadow.mapSize.height = 512;

			// light2.shadow.camera.near = 100;
			// light2.shadow.camera.far = 1200;

			// light2.shadow.camera.left = -1000;
			// light2.shadow.camera.right = 1000;
			// light2.shadow.camera.top = 350;
			// light2.shadow.camera.bottom = -350;

			// scene.add( light2 );






			
			// scene.add( new THREE.CameraHelper( light.shadow.camera ) );


			// var targetObject = new THREE.Object3D();
			// scene.add(targetObject);
			// light.target = targetObject;







			//  GROUND
			var gt = new THREE.TextureLoader().load( "static/img/grasslight-big.jpg" );
			var gg = new THREE.PlaneBufferGeometry( 16000, 16000 );
			var gm = new THREE.MeshPhongMaterial( { color: 0xffffff, map: gt } );

			var ground = new THREE.Mesh( gg, gm );
			ground.rotation.x = - Math.PI / 2;
			ground.material.map.repeat.set( 64, 64 );
			ground.material.map.wrapS = THREE.RepeatWrapping;
			ground.material.map.wrapT = THREE.RepeatWrapping;
				// note that because the ground does not cast a shadow, .castShadow is left false
			ground.receiveShadow = true;

			scene.add( ground );








			// CONTROLS
			cameraControls = new THREE.OrbitControls( camera, renderer.domElement );
			cameraControls.target.set( 0, 50, 0 );
			cameraControls.update();











			events = {
				init: [],
				start: [],
				stop: [],
				keydown: [],
				keyup: [],
				mousedown: [],
				mouseup: [],
				mousemove: [],
				touchstart: [],
				touchend: [],
				touchmove: [],
				update: []
			};

			var scriptWrapParams = 'player,renderer,scene,camera';
			var scriptWrapResultObj = {};

			for ( var eventKey in events ) {

				scriptWrapParams += ',' + eventKey;
				scriptWrapResultObj[ eventKey ] = eventKey;

			}

			var scriptWrapResult = JSON.stringify( scriptWrapResultObj ).replace( /\"/g, '' );

			for ( var uuid in json.scripts ) {

				var object = scene.getObjectByProperty( 'uuid', uuid, true );

				if ( object === undefined ) {

					console.warn( 'APP.Player: Script without object.', uuid );
					continue;

				}

				var scripts = json.scripts[ uuid ];

				for ( var i = 0; i < scripts.length; i ++ ) {

					var script = scripts[ i ];

					var functions = ( new Function( scriptWrapParams, script.source + '\nreturn ' + scriptWrapResult + ';' ).bind( object ) )( this, renderer, scene, camera );

					for ( var name in functions ) {

						if ( functions[ name ] === undefined ) continue;

						if ( events[ name ] === undefined ) {

							console.warn( 'APP.Player: Event type not supported (', name, ')' );
							continue;

						}

						events[ name ].push( functions[ name ].bind( object ) );

					}

				}

			}

			dispatch( events.init, arguments );

		};

















		this.setCamera = function ( value ) {

			camera = value;
			camera.aspect = this.width / this.height;
			camera.updateProjectionMatrix();

			if ( isVR === true ) {

				cameraVR = new THREE.PerspectiveCamera(90, window.innerWidth / window.innerHeight, 1, 4000);
				cameraVR.projectionMatrix = camera.projectionMatrix;
				camera.position.set( 0, 150, 1300 );
				camera.add( cameraVR );


				// CAMERA
				// camera = new THREE.PerspectiveCamera( 45, window.innerWidth / window.innerHeight, 1, 4000 );
				// camera.position.set( 0, 150, 1300 );




				controls = new THREE.VRControls( cameraVR );
				effect = new THREE.VREffect( renderer );

				if ( WEBVR.isAvailable() === true ) {

					this.dom.appendChild( WEBVR.getButton( effect ) );

				}

				if ( WEBVR.isLatestAvailable() === false ) {

					this.dom.appendChild( WEBVR.getMessage() );

				}

			}

		};



			








		this.setScene = function ( value ) {

			scene = value;
			scene.background = new THREE.Color( 0xffffff );
			scene.fog = new THREE.Fog( 0xffffff, 1000, 4000 );


			// // SCENE

			// scene = new THREE.Scene(value);
			// scene.background = new THREE.Color( 0xffffff );
			// scene.fog = new THREE.Fog( 0xffffff, 1000, 4000 );

			// scene.add( camera );

		};




		this.socket_service = function () {
			
			socket.emit('connection', {message: id+" connected"}); 



			socket.on('message', function (data) {
			   	console.log("listen socket.io:: "+data.message);
			});




			socket.emit('send', { message: "simulator app" }); 






			socket.emit('disconnect', {message: id+" disconnected"}); 
		}







		this.setSize = function ( width, height ) {

			this.width = width;
			this.height = height;

			if ( camera ) {

				camera.aspect = this.width / this.height;
				camera.updateProjectionMatrix();

			}

			if ( renderer ) {

				renderer.setSize( width, height );

			}

		};











		function dispatch( array, event ) {

			for ( var i = 0, l = array.length; i < l; i ++ ) {

				array[ i ]( event );

			}

		}














		var prevTime, request;

		function animate( time ) {

			request = requestAnimationFrame( animate );

			try {

				dispatch( events.update, { time: time, delta: time - prevTime } );

			} catch ( e ) {

				console.error( ( e.message || e ), ( e.stack || "" ) );

			}

			if ( isVR === true ) {

				camera.updateMatrixWorld();

				controls.update();
				effect.render( scene, cameraVR );

			} else {

				renderer.render( scene, camera );

			}

			prevTime = time;

		}

















		this.play = function () {

			document.addEventListener( 'keydown', onDocumentKeyDown );
			document.addEventListener( 'keyup', onDocumentKeyUp );
			document.addEventListener( 'mousedown', onDocumentMouseDown );
			document.addEventListener( 'mouseup', onDocumentMouseUp );
			document.addEventListener( 'mousemove', onDocumentMouseMove );
			document.addEventListener( 'touchstart', onDocumentTouchStart );
			document.addEventListener( 'touchend', onDocumentTouchEnd );
			document.addEventListener( 'touchmove', onDocumentTouchMove );

			document.addEventListener( 'SocketMouseRotate', onSocketMouseRotate );


			dispatch( events.start, arguments );

			request = requestAnimationFrame( animate );
			prevTime = performance.now();


		};













		this.stop = function () {

			document.removeEventListener( 'keydown', onDocumentKeyDown );
			document.removeEventListener( 'keyup', onDocumentKeyUp );
			document.removeEventListener( 'mousedown', onDocumentMouseDown );
			document.removeEventListener( 'mouseup', onDocumentMouseUp );
			document.removeEventListener( 'mousemove', onDocumentMouseMove );
			document.removeEventListener( 'touchstart', onDocumentTouchStart );
			document.removeEventListener( 'touchend', onDocumentTouchEnd );
			document.removeEventListener( 'touchmove', onDocumentTouchMove );

			dispatch( events.stop, arguments );

			cancelAnimationFrame( request );

		};















		this.dispose = function () {

			while ( this.dom.children.length ) {

				this.dom.removeChild( this.dom.firstChild );

			}

			renderer.dispose();

			camera = undefined;
			scene = undefined;
			renderer = undefined;

		};












		//

		function onDocumentKeyDown( event ) {

			dispatch( events.keydown, event );

		}

		function onDocumentKeyUp( event ) {

			dispatch( events.keyup, event );

		}

		function onDocumentMouseDown( event ) {

			dispatch( events.mousedown, event );

		}

		function onDocumentMouseUp( event ) {

			dispatch( events.mouseup, event );

		}

		function onDocumentMouseMove( event ) {

			dispatch( events.mousemove, event );

		}




		function onSocketMouseRotate( event ) {
			socket.on('mouseRotate', function (data) {
				dispatch( data, event );
				console.log(data);
			});
		}



		function onDocumentTouchStart( event ) {

			dispatch( events.touchstart, event );

			socket.emit('rotateCanvas', { data: events.mousemove }); 

			console.log(events.mousemove);

		}

		function onDocumentTouchEnd( event ) {

			dispatch( events.touchend, event );

		}

		function onDocumentTouchMove( event ) {

			dispatch( events.touchmove, event );


		}

	}

};
