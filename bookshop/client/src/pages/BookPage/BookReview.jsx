import React, { useState, useEffect } from 'react';
import { Box, Avatar, Typography, Rating } from '@mui/material';
//import { getUser } from '../../mocks/userService';

const Review = ({ review }) => {
    const [user, setUser] = useState(null);
    useEffect(() => {
        fetch(`http://127.0.0.1:8000/getUser/${review.userId}`)
            .then((response) => response.json())
            .then((user) => {
                setUser(user);
            });
    }, [review.userId]);

    let user_image = user ? user.image : "";
    console.log(user_image);
    let user_name = user ? user.name : "";

    return (
        <Box sx={{ padding: 2, position: 'relative' }}>
            <Box sx={{ display: 'flex', alignItems: 'center', mb: 1 }}>
                <Avatar alt={user_name} src={user_image} sx={{ mr: 2, }} />
                <Typography variant="body1" fontWeight="bold">{user_name}</Typography>
            </Box>
            <Typography variant="body2" sx={{ textAlign: 'left' }}>{review.text}</Typography>

            <Box sx={{
                position: 'absolute',
                top: 10,
                right: 10,
                zIndex: 1,
            }}>
                <Rating value={review.rating} readOnly />
            </Box>
        </Box>
    );
};

export default Review;