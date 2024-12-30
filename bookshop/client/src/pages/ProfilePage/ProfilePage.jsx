import React from 'react';
import { Button, Card, CardContent, Typography } from '@mui/material';
import { useAuth } from '../../contexts/AuthContext';
import { jwtDecode } from 'jwt-decode';
import { useNavigate } from 'react-router-dom';

const ProfilePage = () => {
    const { token, saveToken } = useAuth();
    const [user, setUser] = React.useState({});
    const navigate = useNavigate();

    React.useEffect(() => {
        if (token) {
            const decoded = jwtDecode(token);
            fetch(`http://127.0.0.1:8000/getUser/${decoded.user_id}`)
                .then((response) => response.json())
                .then((user) => {
                    setUser(user);
                });
        }
    }, [token]);

    const handleLogout = () => {
        localStorage.removeItem('refresh_token');
        saveToken(null);
        navigate('/login');
    };

    return (
        <div style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', height: '100vh' }}>
            <Card sx={{ maxWidth: 345 }}>
                <CardContent>
                    <Typography gutterBottom variant="h5" component="div">
                        User Profile
                    </Typography>
                    <Typography variant="body2" color="text.secondary">
                        Name: {user.name}
                    </Typography>
                    <Button
                        variant="contained"
                        color="secondary"
                        onClick={handleLogout}
                        sx={{ marginTop: 2 }}
                    >
                        Выйти
                    </Button>
                </CardContent>
            </Card>
        </div>
    );
};

export default ProfilePage;