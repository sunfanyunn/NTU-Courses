package com.example.user.pike;



import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.support.annotation.NonNull;
import android.support.design.widget.NavigationView;
import android.support.v4.app.FragmentActivity;
import android.os.Bundle;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v4.widget.TextViewCompat;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.MenuItemHoverListener;
import android.support.v7.widget.SwitchCompat;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.ToggleButton;

import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.OnMapReadyCallback;
import com.google.android.gms.maps.SupportMapFragment;
import com.google.android.gms.maps.model.BitmapDescriptorFactory;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.LatLngBounds;
import com.google.android.gms.maps.model.Marker;
import com.google.android.gms.maps.model.MarkerOptions;
//import com.example.user.pike.CustomInfoWindowAdapter;
import java.util.ArrayList;
import android.support.v7.widget.SearchView;
import static com.example.user.pike.R.styleable.SearchView;
import static com.example.user.pike.R.styleable.View;

public class MapsActivity extends AppCompatActivity implements GoogleMap.OnInfoWindowClickListener, GoogleMap.OnMarkerClickListener,OnMapReadyCallback, NavigationView.OnNavigationItemSelectedListener {

    private GoogleMap mMap;
    protected NavigationView navigationView;
    private DrawerLayout mdrawerLayout;
    private ActionBarDrawerToggle mToggle;
    private CompoundButton aToggle;
    private TextViewCompat mTextView;
    private Marker[] m = new Marker[10];
    private int[] left = new int[10];
    private Marker park_here;
    private int pike;
    ArrayAdapter<String> adapter;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_maps);
        //getSupportActionBar().hide();
        /*
        ListView lv = (ListView)findViewById(R.id.ListView);
        ArrayList<String> arrayPlaces = new ArrayList<>();
        for(String p : getResources().getStringArray(R.array.array_places))
            arrayPlaces.add(p);

        adapter = new ArrayAdapter<String>(MapsActivity.this, android.R.layout.simple_list_item_1, arrayPlaces);
        lv.setAdapter(adapter);
        lv.setVisibility(ListView.INVISIBLE);
        */

        mdrawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);
        mToggle = new ActionBarDrawerToggle(this, mdrawerLayout, R.string.open, R.string.close);

        mdrawerLayout.addDrawerListener(mToggle);
        mToggle.syncState();

        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        // Obtain the SupportMapFragment and get notified when the map is ready to be used.
        SupportMapFragment mapFragment = (SupportMapFragment) getSupportFragmentManager()
                .findFragmentById(R.id.map);
        mapFragment.getMapAsync(this);


        navigationView = (NavigationView) findViewById(R.id.nav_view);

        navigationView.setNavigationItemSelectedListener(new  NavigationView.OnNavigationItemSelectedListener() {
            @Override
            public boolean onNavigationItemSelected(MenuItem item) {
                // Handle navigation view item clicks here.
                int id = item.getItemId();

                if (id == R.id.nav_locate) {

                    //private GoogleMap mMap;
                    // Create a LatLngBounds that includes Australia.
                    LatLng tmp = park_here.getPosition();
                    double sz = 0.003;
                    LatLngBounds AUSTRALIA = new LatLngBounds(
                            new LatLng(tmp.latitude-sz, tmp.longitude-sz) , new LatLng(tmp.latitude+sz, tmp.longitude+sz) );

                    // Set the camera to the greatest possible zoom level that includes the
                    // bounds
                    mMap.moveCamera(CameraUpdateFactory.newLatLngBounds(AUSTRALIA, 0));

                    m[pike].setVisible(false);
                    park_here.setVisible(true);
                    park_here.showInfoWindow();

                } else if( id == R.id.nav_people) {
                    Intent viewIntent =
                            new Intent("android.intent.action.VIEW",
                                    Uri.parse("https://www.csie.ntu.edu.tw/~b04902045/hcid/#team"));
                    startActivity(viewIntent);
                }else if( id == R.id.nav_report) {

                } else if( id == R.id.nav_school ) {

                            Intent intent  = new Intent();
                            intent.setClass(MapsActivity.this, StartingActivity.class);
                            startActivity(intent);

                }

                DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
                drawer.closeDrawer(GravityCompat.START);
                return true;
            }
        });


    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        /*
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu, menu);
        MenuItem item =  menu.findItem(R.id.menuSearch);

        SearchView searchview = (SearchView)item.getActionView();
        searchview.setOnQueryTextListener( new SearchView.OnQueryTextListener() {
            @Override
            public boolean onQueryTextSubmit(String query) {
                return false;
            }

            @Override
            public boolean onQueryTextChange(String newText) {
                //adapter.getFilter().filter(newText);
                return false;
            }

        });
        */
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onNavigationItemSelected(@NonNull MenuItem item) {
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (mToggle.onOptionsItemSelected(item)) return true;
        return super.onOptionsItemSelected(item);
    }

    void park_here(int p) {
        park_here.setVisible(false);
        m[pike].setVisible(true);

        pike = p;
        m[p].setVisible(false);

        park_here.setPosition(m[p].getPosition());
        park_here.setSnippet(m[p].getTitle());
        park_here.setVisible(true);
        left[p]--;
    }
    /**
     * Manipulates the map once available.
     * This callback is triggered when the map is ready to be used.
     * This is where we can add markers or lines, add listeners or move the camera. In this case,
     * we just add a marker near Sydney, Australia.
     * If Google Play services is not installed on the device, the user will be prompted to install
     * it inside the SupportMapFragment. This method will only be triggered once the user has
     * installed Google Play services and returned to the app.
     */
    @Override
    public void onMapReady(GoogleMap googleMap) {
        mMap = googleMap;

        mMap.setMapType(GoogleMap.MAP_TYPE_NORMAL);
        mMap.getUiSettings().setZoomControlsEnabled(true);

        //Set Custom InfoWindow Adapter
        CustomInfoWindowAdapter adapter = new CustomInfoWindowAdapter(MapsActivity.this);
        mMap.setInfoWindowAdapter(adapter);


        // Add a marker in Sydney and move the camera
        LatLng ntu = new LatLng(25.016, 121.538);
         mMap.addMarker(new MarkerOptions().position(ntu).title("You are here!"));
        mMap.moveCamera(CameraUpdateFactory.newLatLngZoom(ntu, 15.0f));


        left[0] = 13;
        m[0] = mMap.addMarker(new MarkerOptions()
                .position(new LatLng(25.02, 121.54))
                .title("Freshman Building")
                .snippet(Integer.toString(left[0]) + " spaces left\nClick me to park here!")
                .icon(BitmapDescriptorFactory.fromResource(R.mipmap.available)) );


        left[1] = 0;
        m[1] = mMap.addMarker(new MarkerOptions()
                .position(new LatLng(25.018, 121.537))
                .title("Main Library")
                .snippet("0 parking spaces left\nClick me to park here!")
                .icon(BitmapDescriptorFactory.fromResource(R.mipmap.unavailable)) );

        left[2] = 9;
        m[2] = mMap.addMarker(new MarkerOptions()
                .position(new LatLng(25.017611, 121.540012))
                .title("Student Activity Center")
                .snippet(Integer.toString(left[2])+" spaces left\nClick me to park here!")
                .icon(BitmapDescriptorFactory.fromResource(R.mipmap.available)) );
        left[3] = 0;
        m[3] = mMap.addMarker(new MarkerOptions()
                .position(new LatLng(25.016119, 121.534766))
                .title("First Women's Dorm")
                .snippet("0 parking spaces left\nClick me to park here!")
                .icon(BitmapDescriptorFactory.fromResource(R.mipmap.unavailable)) );
        pike = 0;
        park_here = mMap.addMarker(new MarkerOptions()
                .position(m[pike].getPosition())
                .title("You park here!")
                .icon(BitmapDescriptorFactory.fromResource(R.mipmap.bike))
                .visible(false)
        );



       // mTextView = (TextView)findViewById(R.id.textView2);


        aToggle = (Switch) findViewById(R.id.switch2);
        aToggle.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    // The toggle is enabled

                    park_here.setVisible(false);
                    m[0].setVisible(true);
                    m[2].setVisible(true);
                    m[1].setVisible(true);
                    m[3].setVisible(true);
                    aToggle.setText("All");
                } else {

                    park_here.setVisible(false);
                    m[0].setVisible(true);
                    m[2].setVisible(true);
                    m[1].setVisible(false);
                    m[3].setVisible(false);
                    aToggle.setText("Available");

                }
            }
        });

        mMap.setOnMarkerClickListener(new GoogleMap.OnMarkerClickListener() {
            @Override
            public boolean onMarkerClick(Marker marker) {

                return false;
            }
        });
        mMap.setOnInfoWindowClickListener( new GoogleMap.OnInfoWindowClickListener() {
            @Override
            public void onInfoWindowClick(Marker marker)  {
                for(int i = 0; i < 4; i++)
                    if( marker.getPosition().latitude == m[i].getPosition().latitude && marker.getPosition().longitude == m[i].getPosition().longitude) {
                        park_here(i);
                    }
            }
        });
    }
    @Override
    public boolean onMarkerClick(Marker marker) {
        return false;
    }
    @Override
    public void onInfoWindowClick(Marker marker) {
    }

    public class CustomInfoWindowAdapter implements GoogleMap.InfoWindowAdapter {

        private Activity context;

        public CustomInfoWindowAdapter(Activity context){
            this.context = context;
        }

        @Override
        public View getInfoWindow(Marker marker) {
            return null;
        }

        @Override
        public View getInfoContents(Marker marker) {
            View view = context.getLayoutInflater().inflate(R.layout.custominfowindow, null);

            TextView tvTitle = (TextView) view.findViewById(R.id.tv_title);
            TextView tvSubTitle = (TextView) view.findViewById(R.id.tv_subtitle);

            tvTitle.setText(marker.getTitle());
            tvSubTitle.setText(marker.getSnippet());

            return view;
        }
    }
}


